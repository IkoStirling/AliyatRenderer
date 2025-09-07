#include "AYSoundEngine.h"
#include "AYPath.h"
#include "Event_CameraMove.h"

AYSoundEngine::AYSoundEngine():
    _preloadPath("@audios/AudioCachePreload.json")
{
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
}

AYSoundEngine::~AYSoundEngine()
{
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(nullptr);
    if (context) alcDestroyContext(context);
    if (device) alcCloseDevice(device);
}

void AYSoundEngine::init()
{
    if (_initialized) return;

    auto system = GET_CAST_MODULE(AYEventSystem, "EventSystem");
    if (system)
    {
        auto token = system->subscribe(Event_CameraMove::staticGetType(),
            [this](const IAYEvent& in_event) {
                auto& event = static_cast<const Event_CameraMove&>(in_event);
                auto& trans = event.transform;
                auto& tp = trans.position;
                //std::cout << "[AYSoundEngine] update sound location[" << tp.x << ", " << tp.y << ", " << tp.z << "]\n";
                setListenerPosition(trans.position, trans.getForwardVector(), trans.getUpVector());
            });
        _tokens.push_back(std::unique_ptr<AYEventToken>(token));
    }
    _preloadAudios();
    _initialized = true;
}

void AYSoundEngine::update(float delta_time)
{
    std::lock_guard<std::mutex> lock(_cacheMutex);

    // 更新监听器位置
    alListener3f(AL_POSITION, _listenerPosition.x, _listenerPosition.y, _listenerPosition.z);

    float orientation[6] = {
        _listenerForward.x, _listenerForward.y, _listenerForward.z,
        _listenerUp.x, _listenerUp.y, _listenerUp.z
    };
    alListenerfv(AL_ORIENTATION, orientation);

    for (auto& player : _players)
    {
        player->update();
    }

    // 清理已完成播放的源
    _activeAudios.erase(
        std::remove_if(_activeAudios.begin(), _activeAudios.end(),
            [](const ActiveAudioSource& source) {
                if (source.audio.expired()) return true;
                if (source.player->isPlaybackFinished()) return true;
                return false;
            }),
        _activeAudios.end());

    for (auto& video : _activeVideos) {
        if (video->isPlaying()) {
            video->updateFrame(delta_time);

            // 检查视频是否结束
            if (video->isEndOfVideo()) {
                // 处理循环或结束逻辑
            }
        }
    }

    // 移除已完成视频
    _activeVideos.erase(std::remove_if(_activeVideos.begin(), _activeVideos.end(),
        [](const auto& v) { return v->isEndOfVideo(); }),
        _activeVideos.end());

}

void AYSoundEngine::shutdown()
{
    stopAll();

    _saveAudios();

    _audioCache.clear();

    _tokens.clear();

    _initialized = false;
    std::cout << "[AYSoundEngine] shutdown complete\n";
}


std::shared_ptr<IAYAudioSource> AYSoundEngine::playSound2D(const std::string& path, bool isStreaming, bool loop, float volume, bool asyncload)
{
    return playSound3D(path, glm::vec3(0, 0, 0), isStreaming, loop, volume, asyncload);
}

std::shared_ptr<IAYAudioSource> AYSoundEngine::playSound3D(const std::string& path, const glm::vec3& position, bool isStreaming, bool loop, float volume, bool asyncload)
{
    if (!_initialized) return nullptr;

    std::shared_ptr<IAYAudioSource> audio;

    // 检查缓存
    {
        std::lock_guard<std::mutex> cacheLock(_cacheMutex);
        auto it = _audioCache.find(path);
        if (it != _audioCache.end()) {
            audio = it->second.lock();
            if (audio) {
                // 缓存命中，直接播放
                _playAudioImpl(audio, position, loop, volume, position != glm::vec3(0));
                return audio;
            }
        }
    }

    auto& rm = AYResourceManager::getInstance();

    if (!audio) {
        if (asyncload) {
            // 异步加载，可以确保this指针在调用时存在
            // 3D默认为静态音频
            if (isStreaming)
            {
                rm.loadAsync<AYAudioStream>(path,
                    [this, position, loop, volume, path](std::shared_ptr<AYAudioStream> loadedAudio) {
                        if (loadedAudio) {
                            std::lock_guard<std::mutex> lock(_cacheMutex);
                            _audioCache[path] = loadedAudio;
                            _playAudioImpl(loadedAudio, position, loop, volume, position != glm::vec3(0, 0, 0));
                        }
                    });
            }
            else
            {
                rm.loadAsync<AYAudio>(path,
                    [this, position, loop, volume, path](std::shared_ptr<AYAudio> loadedAudio) {
                        if (loadedAudio) {
                            std::lock_guard<std::mutex> lock(_cacheMutex);
                            _audioCache[path] = loadedAudio;
                            _playAudioImpl(loadedAudio, position, loop, volume, position != glm::vec3(0, 0, 0));
                        }
                    });
            }
            
            return nullptr;
        }
        else {
            // 同步加载
            if (isStreaming)
                audio = rm.load<AYAudioStream>(path);
            else
                audio = rm.load<AYAudio>(path);

            if (!audio) return nullptr;

            std::lock_guard<std::mutex> lock(_cacheMutex);
            _audioCache[path] = audio;
        }
    }

    _playAudioImpl(audio, position, loop, volume, position != glm::vec3(0, 0, 0));
    return audio;
}

std::shared_ptr<AYVideo> AYSoundEngine::playVideo(const std::string& path, const std::shared_ptr<IAYAudioSource>& audio, bool loop)
{
    if (!_initialized) return nullptr;

    std::shared_ptr<AYVideo> video;

    auto& rm = AYResourceManager::getInstance();
    video = rm.load<AYVideo>(path);

    if (!video) return nullptr;

    if (audio) {
        _syncVideoToAudio(video, audio);
    }
    else
    {
        if (video->hasAudio())
        {
            auto audio = video->getAudio();
            _syncVideoToAudio(video, audio);
            _playAudioImpl(audio, glm::vec3(0, 0, 0), loop, 1.f, false);
        }
    }

    video->play();
    _activeVideos.push_back(video);
    return video;
}

void AYSoundEngine::setMasterVolume(float volume)
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    float _oldMasterInv = 1 / _masterVolume;
    _masterVolume = std::clamp(volume, 0.0f, 1.0f);

    for (auto& player : _players) {
        player->setMasterVolume(volume);
    }
}

void AYSoundEngine::pauseAll()
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    for (auto& source : _activeAudios) {
        source.player->pause();
    }
}

void AYSoundEngine::resumeAll()
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    for (auto& source : _activeAudios) {
        source.player->resume();
    }
}

void AYSoundEngine::stopAll()
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    for (auto& source : _activeAudios) {
        source.player->stop();
    }
    _activeAudios.clear();
}

void AYSoundEngine::seek(const std::string& path, float seconds)
{
    // 查找对应的音频和视频资源
    std::shared_ptr<IAYAudioSource> audio;
    std::shared_ptr<AYVideo> video;

    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        if (_audioCache.find(path) != _audioCache.end()) {
            audio = _audioCache[path].lock();
        }
        if (_videoCache.find(path) != _videoCache.end()) {
            video = _videoCache[path].lock();
        }
    }

    if (audio) {
        if (auto player = std::dynamic_pointer_cast<AYAudioPlayer>(audio)) {
            player->seek(seconds);
        }
    }

    if (video) {
        video->seekToTime(seconds);
    }
}

void AYSoundEngine::setSourcePosition(std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& position)
{
    std::lock_guard<std::mutex> lock(_sourceMutex);

    // 查找音源
    auto it = std::find_if(_activeAudios.begin(), _activeAudios.end(),
        [audio](const ActiveAudioSource& s) { return s.audio.lock() == audio; });

    if (it != _activeAudios.end()) {
        it->player->setPosition(position);
    }
}

void AYSoundEngine::setSourceVelocity(std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& velocity)
{
    std::lock_guard<std::mutex> lock(_sourceMutex);

    auto it = std::find_if(_activeAudios.begin(), _activeAudios.end(),
        [audio](const ActiveAudioSource& s) { return s.audio.lock() == audio; });

    if (it != _activeAudios.end()) {
        it->player->setVelocity(velocity);
    }
}

void AYSoundEngine::setAttenuationParameter(float rolloff_factor, float reference_distance, float max_distance)
{
    _rolloffFactor = rolloff_factor;
    _referenceDistance = reference_distance;
    _maxDistance = max_distance;

    for (auto player : _players)
        player->set3DParameters(_rolloffFactor, _referenceDistance, _maxDistance);
}

void AYSoundEngine::setListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    _listenerPosition = position;
    _listenerForward = glm::normalize(forward);
    _listenerUp = glm::normalize(up);
}

void AYSoundEngine::_preloadAudios()
{
    _preloadFile.loadFromFile(_preloadPath, AYConfigWrapper::ConfigType::JSON);

    auto& rm = AYResourceManager::getInstance();
    for (const auto& path : _preloadFile.getVector<std::string>("audio_preload"))
    {
        rm.loadAsync<AYAudio>(path, [this,path](std::shared_ptr<AYAudio> loadedAudio){
            std::lock_guard<std::mutex> lock(_cacheMutex);
            _audioCache[path] = loadedAudio;
            });
    }
}

void AYSoundEngine::_saveAudios()
{
    std::vector<std::string> paths;
    for (const auto& [path, audio] : _audioCache)
    {
        if(!audio.lock()->isStreaming())
            paths.push_back(path);
    }
    _preloadFile.set<std::string>("audio_preload", paths);
    _preloadFile.saveConfig(_preloadPath);
}

void AYSoundEngine::_playAudioImpl(const std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& position, bool loop, float volume, bool is3D)
{
    std::shared_ptr<AYAudioPlayer> splayer;
    for (auto player : _players)
    {
        if (player->isAvaliableOrInterruptible())
        {
            splayer = player;
            break;
        }
    }

    if (!splayer)
    {
        if (_players.size() < maxAudioPlayerNum)
        {
            _players.push_back(std::make_shared<AYAudioPlayer>());
            splayer = _players.back();
            splayer->set3DParameters(_rolloffFactor, _referenceDistance, _maxDistance);
        }
        else
            return;
    }

    if (audio->getChannels() == 2)
        is3D = false;

    splayer->set3DEnabled(is3D);
    splayer->setVolume(volume);
    splayer->play(audio, loop);

    // 添加到活动源列表
    std::lock_guard<std::mutex> lock(_sourceMutex);
    _activeAudios.push_back({
    audio,
    splayer,
    loop, // 循环音频标记为持久
    time(nullptr)
    });

}

void AYSoundEngine::_syncVideoToAudio(const std::shared_ptr<AYVideo>& video, const std::shared_ptr<IAYAudioSource>& audio)
{
    video->setSyncCallback([audio](AYVideo& v) {
        if (auto player = dynamic_cast<AYAudioPlayer*>(audio.get())) {
            double audioTime = player->getCurrentTime();
            v.syncToAudio(audioTime);
        }
        });
}