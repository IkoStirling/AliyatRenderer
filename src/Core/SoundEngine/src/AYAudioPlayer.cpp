#include "AYAudioPlayer.h"
#include "AYAudioPlayer.h"
#include "AYAudioStream.h"
#include <thread>
#include <memory>

AYAudioPlayer::AYAudioPlayer(bool enable3D) : _is3DEnabled(enable3D) {
    alGenSources(1, &_source);
    updateSourceProperties();
}

AYAudioPlayer::~AYAudioPlayer() {
    stop();
    alDeleteSources(1, &_source);
}

bool AYAudioPlayer::play(const std::shared_ptr<IAYAudioSource>& source, bool loop) {
    stop(); // 停止当前播放
    if (!source || !source->getSampleRate()) return false;

    _currentSource = source;
    _loop.store(loop);
    _state.store(PlayState::Playing);

    // 静态音频直接提交全部数据
    if (!source->isStreaming()) {
        ALuint buffer;
        alGenBuffers(1, &buffer);

        float sourceGain = source->getSuggestedGain(); // 默认返回1.0f
        // 应用动态范围压缩
        const float compressionFactor = 0.8f; // 轻度压缩
        alBufferData(buffer, source->getFormat(),
            source->getPCMData().data(),
            (ALsizei)source->getPCMData().size(),
            source->getSampleRate());

        // 设置缓冲区独立增益
        if (alIsExtensionPresent("AL_SOFT_buffer_gain")) {
            alBufferf(buffer, AL_GAIN, sourceGain * compressionFactor);
        }
        else {
            _volume.store(sourceGain * compressionFactor * _volume.load());
        }

        alSourcei(_source, AL_BUFFER, buffer);
        alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        updateSourceProperties();
        alSourcePlay(_source);
    }
    // 流式音频需要缓冲
    else {
        if (!refillBuffers(3)) { // 预缓冲3个区块
            return false;
        }
        updateSourceProperties();
        alSourcePlay(_source);
    }

    return true;
}

void AYAudioPlayer::pause()
{
    if (_state == PlayState::Playing) {
        alSourcePause(_source);
        _state.store(PlayState::Paused);
    }
}
void AYAudioPlayer::resume()
{
    if (_state == PlayState::Paused) {
        alSourcePlay(_source);
        _state.store(PlayState::Playing);
    }
}



void AYAudioPlayer::stop() {
    if (_state == PlayState::Stopped) return;

    alSourceStop(_source);
    cleanup();
    _state.store(PlayState::Stopped);

    _callback = nullptr;
}

void AYAudioPlayer::seek(float seconds)
{
    if (!_currentSource) return;

    std::lock_guard<std::mutex> lock(_bufferMutex);

    if (_currentSource->isStreaming()) {
        // 对于流式音频，需要清空缓冲区并重新填充
        cleanup();
        auto source = std::static_pointer_cast<AYAudioStream>(_currentSource);
        source->seekToTime(seconds);
        refillBuffers(3);
    }
    else {
        // 对于静态音频，直接设置播放位置
        alSourcef(_source, AL_SEC_OFFSET, seconds);
    }
}

void AYAudioPlayer::setPlaybackFinishedCallback(PlaybackFinishedCallback callback)
{
    _callback = callback;
}

void AYAudioPlayer::setMasterVolume(float volume) {
    _masterVolume.store(std::clamp(volume, 0.0f, 1.0f));
    updateSourceProperties();
}

void AYAudioPlayer::setVolume(float volume)
{
    _volume.store(std::clamp(volume, 0.0f, 1.0f));
    updateSourceProperties();
}

void AYAudioPlayer::setPosition(const glm::vec3& position)
{
    _position = position;
    if (_is3DEnabled) {
        updateSourceProperties();
    }
}

void AYAudioPlayer::setVelocity(const glm::vec3& velocity)
{
    _velocity = velocity;
    if (_is3DEnabled) {
        updateSourceProperties();
    }
}

void AYAudioPlayer::setLoop(bool loop)
{
    _loop.store(loop);
    // 静态音频立即设置，流式音频在update中处理
    if (!_currentSource || !_currentSource->isStreaming()) {
        alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    }
}

void AYAudioPlayer::set3DEnabled(bool enabled)
{
    _is3DEnabled = enabled;
    updateSourceProperties();
}

void AYAudioPlayer::set3DParameters(float rolloff, float refDistance, float maxDistance)
{
    _rolloffFactor = rolloff;
    _referenceDistance = refDistance;
    _maxDistance = maxDistance;

    updateSourceProperties();
}

bool AYAudioPlayer::isPlaybackFinished() const {
    // 1. 基础条件检查
    if (!_currentSource || _state != PlayState::Playing) {
        return false;
    }

    // 2. 静态音频检查
    if (!_currentSource->isStreaming()) {
        ALint state;
        alGetSourcei(_source, AL_SOURCE_STATE, &state);
        return (state == AL_STOPPED && !_loop.load());
    }
    // 3. 流式音频检查
    else {
        ALint state;
        alGetSourcei(_source, AL_SOURCE_STATE, &state);

        // 获取缓冲区状态
        ALint processed;
        alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);
        ALint queued;
        alGetSourcei(_source, AL_BUFFERS_QUEUED, &queued);

        // 完成条件：播放停止 + 无剩余有效缓冲区 + 不循环
        return (state == AL_STOPPED) &&
            (processed == queued) &&
            !_loop.load();
    }
}

bool AYAudioPlayer::isAvaliableOrInterruptible() const {
    return _state == PlayState::Stopped ||
        (_state == PlayState::Playing && _loop.load());
}

AYAudioPlayer::PlayState AYAudioPlayer::getState() const
{
    return _state.load();
}

float AYAudioPlayer::getCurrentTime() const
{
    if (!_currentSource) return 0.0f;

    ALfloat offset;
    alGetSourcef(_source, AL_SEC_OFFSET, &offset);
    return offset;
}

bool AYAudioPlayer::isStreaming() const
{
    return _currentSource && _currentSource->isStreaming();
}

void AYAudioPlayer::update() {
    if (_state != PlayState::Playing || !_currentSource) return;

    // 处理流式音频缓冲
    if (_currentSource->isStreaming()) {
        ALint processed;
        alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);

        // 回收已播放的缓冲区
        while (processed--) {
            ALuint buffer;
            alSourceUnqueueBuffers(_source, 1, &buffer);
            alDeleteBuffers(1, &buffer);
        }

        // 维持至少2个缓冲区块
        if (!refillBuffers(2)) {
            if (_loop.load()) {  // 使用原子变量
                auto source = std::static_pointer_cast<AYAudioStream>(_currentSource);
                source->seekToTime(0);  // 重置到开头
                if (!refillBuffers(3)) {  // 重新填充缓冲区
                    stop();
                }
            }
            else {
                stop();
            }
        }
    }

    // 检查播放是否结束
    bool state = isPlaybackFinished();
    if (state)
    {
        if (_callback) {
            _callback();
        }
    }
}


void AYAudioPlayer::cleanup() {
    // 清空缓冲区队列（仅对流式音频）
    if (_currentSource && _currentSource->isStreaming()) {
        ALint queued;
        alGetSourcei(_source, AL_BUFFERS_QUEUED, &queued);

        while (queued--) {
            ALuint buffer;
            alSourceUnqueueBuffers(_source, 1, &buffer);
            alDeleteBuffers(1, &buffer);
        }
    }
    // 对于静态音频只需解除缓冲区绑定
    else if (_currentSource) {
        alSourcei(_source, AL_BUFFER, 0);
    }

    _currentSource.reset();
}

void AYAudioPlayer::updateSourceProperties() {
    alSourcef(_source, AL_GAIN, _volume.load() * _masterVolume.load());

    if (_is3DEnabled) {
        alSource3f(_source, AL_POSITION, _position.x, _position.y, _position.z);
        alSource3f(_source, AL_VELOCITY, _velocity.x, _velocity.y, _velocity.z);
        alSourcei(_source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(_source, AL_ROLLOFF_FACTOR, 1.0f);
    }
    else {
        alSource3f(_source, AL_POSITION, 0, 0, 0);
        alSourcei(_source, AL_SOURCE_RELATIVE, AL_TRUE);
    }
}

bool AYAudioPlayer::refillBuffers(size_t minFrames) {
    std::lock_guard<std::mutex> lock(_bufferMutex);

    ALint queued;
    alGetSourcei(_source, AL_BUFFERS_QUEUED, &queued);

    const size_t targetFrames = _currentSource->getSampleRate() / 50;

    while (queued < minFrames) {
        auto source = std::static_pointer_cast<AYAudioStream>(_currentSource);
        auto frame = source->decodeNextFrame();
        if (!frame) return false;

        if (frame->data.size() % (2 * _currentSource->getChannels()) != 0) {
            frame->data.resize(frame->data.size() - (frame->data.size() %
                (2 * _currentSource->getChannels())));
        }

        ALuint buffer;
        alGenBuffers(1, &buffer);

        alBufferData(buffer, _currentSource->getFormat(),
            frame->data.data(), (ALsizei)frame->data.size(),
            _currentSource->getSampleRate());

        alSourceQueueBuffers(_source, 1, &buffer);
        queued++;
    }

    // 如果源意外停止，重新启动
    ALint state;
    alGetSourcei(_source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && queued > 0) {
        alSourcePlay(_source);
    }

    return true;
}

