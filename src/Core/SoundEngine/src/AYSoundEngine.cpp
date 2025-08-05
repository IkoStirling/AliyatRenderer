#include "AYSoundEngine.h"
#include "AYPath.h"

AYSoundEngine::AYSoundEngine()
{
}

void AYSoundEngine::init()
{
    if (_initialized) return;

    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open OpenAL device" << std::endl;
        return;
    }

    ALCcontext* context = alcCreateContext(device, nullptr);
    if (!context) {
        std::cerr << "Failed to create OpenAL context" << std::endl;
        alcCloseDevice(device);
        return;
    }

    if (!alcMakeContextCurrent(context)) {
        std::cerr << "Failed to make OpenAL context current" << std::endl;
        alcDestroyContext(context);
        alcCloseDevice(device);
        return;
    }
    _preloadAudios();
    _initialized = true;
}

void AYSoundEngine::update(float delta_time)
{
    std::lock_guard<std::mutex> lock(_mutex);

    // 更新监听器位置
    alListener3f(AL_POSITION, _listenerPosition.x, _listenerPosition.y, _listenerPosition.z);

    float orientation[6] = {
        _listenerForward.x, _listenerForward.y, _listenerForward.z,
        _listenerUp.x, _listenerUp.y, _listenerUp.z
    };
    alListenerfv(AL_ORIENTATION, orientation);

    // 清理已完成播放的源
    _activeSources.erase(
        std::remove_if(_activeSources.begin(), _activeSources.end(),
            [](const ActiveAudioSource& source) {
                ALint state;
                alGetSourcei(source.sourceId, AL_SOURCE_STATE, &state);
                if (state != AL_PLAYING) {
                    alDeleteSources(1, &source.sourceId);
                    return true;
                }
                return false;
            }),
        _activeSources.end());
}

void AYSoundEngine::shutdown()
{
    stopAll();

    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(nullptr);
    if (context) alcDestroyContext(context);
    if (device) alcCloseDevice(device);

    _saveAudios();
    _initialized = false;
}

std::shared_ptr<AYAudio> AYSoundEngine::play2D(const std::string& path, bool loop, float volume, bool asyncLoad)
{
    return play3D(path, glm::vec3(0, 0, 0), loop, volume, asyncLoad);
}

std::shared_ptr<AYAudio> AYSoundEngine::play3D(const std::string& path, const glm::vec3& position, bool loop, float volume, bool asyncLoad)
{
    if (!_initialized) return nullptr;

    std::shared_ptr<AYAudio> audio;

    // 检查缓存
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _audioCache.find(path);
        if (it != _audioCache.end()) {
            audio = it->second.lock();
        }
    }

    if (!audio) {
        auto& rm = AYResourceManager::getInstance();

        if (asyncLoad) {
            // 异步加载
            rm.loadAsync<AYAudio>(path, 
                [this, position, loop, volume](std::shared_ptr<AYAudio> loadedAudio) {
                if (loadedAudio) {
                    this->_playAudioImpl(loadedAudio, position, loop, volume, position != glm::vec3(0, 0, 0));
                }
                });
            return nullptr;
        }
        else {
            // 同步加载
            audio = rm.load<AYAudio>(path);
            if (!audio) return nullptr;

            std::lock_guard<std::mutex> lock(_mutex);
            _audioCache[path] = audio;
        }
    }

    _playAudioImpl(audio, position, loop, volume, position != glm::vec3(0, 0, 0));
    return audio;
}

void AYSoundEngine::setMasterVolume(float volume)
{
}

void AYSoundEngine::pauseAll()
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& source : _activeSources) {
        alSourcePause(source.sourceId);
    }
}

void AYSoundEngine::resumeAll()
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& source : _activeSources) {
        alSourcePlay(source.sourceId);
    }
}

void AYSoundEngine::stopAll()
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& source : _activeSources) {
        alSourceStop(source.sourceId);
        alDeleteSources(1, &source.sourceId);
    }
    _activeSources.clear();
}

void AYSoundEngine::setListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _listenerPosition = position;
    _listenerForward = glm::normalize(forward);
    _listenerUp = glm::normalize(up);
}

void AYSoundEngine::_preloadAudios()
{
    _preloadPath = AYPath::Engine::getPresetAudioPath() + std::string("AudioCachePreload.json");
    _preloadFile.loadFromFile(_preloadPath, AYConfigWrapper::ConfigType::JSON);

    for (const auto& path : _preloadFile.getVector<std::string>("audio_preload"))
    {
        auto& rm = AYResourceManager::getInstance();
        rm.loadAsync<AYAudio>(path, [](std::shared_ptr<AYAudio> loadedAudio){});
    }
}

void AYSoundEngine::_saveAudios()
{
    for (const auto& [path, _] : _audioCache)
    {
        _preloadFile.set<std::string>("audio_preload", path);
    }

    _preloadFile.saveConfig(_preloadPath);
}

void AYSoundEngine::_playAudioImpl(const std::shared_ptr<AYAudio>& audio, const glm::vec3& position, bool loop, float volume, bool is3D)
{
    ALuint sourceId;
    alGenSources(1, &sourceId);

    // 设置源属性
    alSourcei(sourceId, AL_BUFFER, audio->getAudioBuffer());
    alSourcei(sourceId, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcef(sourceId, AL_GAIN, volume * _masterVolume);

    if (is3D) {
        alSource3f(sourceId, AL_POSITION, position.x, position.y, position.z);
        alSourcei(sourceId, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(sourceId, AL_ROLLOFF_FACTOR, 1.0f);
        alSourcef(sourceId, AL_REFERENCE_DISTANCE, 10.0f);
        alSourcef(sourceId, AL_MAX_DISTANCE, 100.0f);
    }
    else {
        alSource3f(sourceId, AL_POSITION, 0, 0, 0);
        alSourcei(sourceId, AL_SOURCE_RELATIVE, AL_TRUE);
    }

    alSourcePlay(sourceId);

    // 添加到活动源列表
    std::lock_guard<std::mutex> lock(_mutex);
    _activeSources.push_back({ audio, sourceId, position, is3D });
}