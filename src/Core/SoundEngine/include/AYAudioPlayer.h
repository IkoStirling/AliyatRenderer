#pragma once
#include "IAYAudioSource.h"
#include "glm/glm.hpp"
#include <mutex>
#include <atomic>
namespace ayt::engine::resource
{
    /*
        同一时间只能播放一个音源
        位于独立线程，与静态音源区分（静态是指内存加载方式，并非播放属性）
    */
    class AYAudioPlayer
    {
    public:
        using PlaybackFinishedCallback = std::function<void()>;
        enum class PlayState {
            Stopped,
            Playing,
            Paused
        };

        explicit AYAudioPlayer(bool enable3D = false);
        ~AYAudioPlayer();

        // 基础控制
        bool play(const std::shared_ptr<IAYAudioSource>& source, bool loop = false);
        void pause();
        void resume();
        void stop();
        void seek(float seconds); // 仅对可定位源有效
        void setPlaybackFinishedCallback(PlaybackFinishedCallback callback);

        // 属性控制
        void setMasterVolume(float volume);
        void setVolume(float volume);
        void setPosition(const glm::vec3& position);
        void setVelocity(const glm::vec3& velocity);
        void setLoop(bool loop);
        void set3DEnabled(bool enabled);

        // 3D音频参数设置
        void set3DParameters(float rolloff, float refDistance, float maxDistance);

        // 状态查询
        bool isPlaybackFinished() const;
        bool isAvaliableOrInterruptible() const;
        PlayState getState() const;
        float getCurrentTime() const;
        bool isStreaming() const;
        ALuint getSourceId() const { return _source; }

        // 更新方法（用于流式音频）
        void update();

        // 获取流式音频
        std::shared_ptr<IAYAudioSource> getAudioSource() { return _currentSource; }

    private:
        void cleanup();
        bool refillBuffers(size_t minFrames = 2);
        void updateSourceProperties();

        PlaybackFinishedCallback _callback;
        ALuint _source = 0;
        std::queue<ALuint> _bufferQueue;
        std::shared_ptr<IAYAudioSource> _currentSource;

        // 状态控制
        std::atomic<PlayState> _state{ PlayState::Stopped };
        std::atomic<bool> _loop{ false };
        std::atomic<float> _volume{ 1.0f };
        std::atomic<float> _masterVolume{ 1.0f };
        glm::vec3 _position{ 0 };
        glm::vec3 _velocity{ 0 };
        bool _is3DEnabled;

        // 3D音频参数
        float _rolloffFactor = 1.0f;
        float _referenceDistance = 20.0f;
        float _maxDistance = 500.0f;

        mutable std::mutex _bufferMutex;
    };
}