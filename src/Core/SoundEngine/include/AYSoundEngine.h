#pragma once
#include "Mod_SoundEngine.h"
#include "AYResourceManager.h"
#include "AYAudio.h"
#include "AYAudioStream.h"
#include "AYAudioPlayer.h"
#include "AYEventSystem.h"
#include "AYConfigWrapper.h"
#include "glm/glm.hpp"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>

class AYSoundEngine : public Mod_SoundEngine
{
public:
    AYSoundEngine();
    ~AYSoundEngine();

    void init() override;
    void shutdown() override;
    void update(float delta_time) override;

    // 音频播放控制
    std::shared_ptr<IAYAudioSource> play2D(const std::string& path,
        bool isStreaming = false,
        bool loop = false,
        float volume = 1.0f,
        bool asyncload = true);

    std::shared_ptr<IAYAudioSource> play3D(const std::string& path,
        const glm::vec3& position,
        bool isStreaming = false,
        bool loop = false,
        float volume = 1.0f,
        bool asyncload = true);

    // 全局控制
    void setMasterVolume(float volume);
    void pauseAll();
    void resumeAll();
    void stopAll();

    // 音源设置
    void setSourcePosition(std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& position);
    void setSourceVelocity(std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& velocity);
    void setAttenuationParameter(float rolloff_factor, float reference_distance, float max_distance);

    // 音频源管理, 已绑定到当前摄像机位置，不需要手动调用
    void setListenerPosition(const glm::vec3& position,
        const glm::vec3& forward = glm::vec3(0, 0, -1),
        const glm::vec3& up = glm::vec3(0, 1, 0));

private:
    struct ActiveAudioSource {
        std::weak_ptr<IAYAudioSource> audio;
        std::shared_ptr<AYAudioPlayer> player;
        bool isPersistent = false; // 标记是否持久化(如背景音乐)
        time_t startTime = 0;      // 开始播放时间
    };  

    void _preloadAudios();
    void _saveAudios();

    void _playAudioImpl(const std::shared_ptr<IAYAudioSource>& audio,
        const glm::vec3& position,
        bool loop,
        float volume,
        bool is3D);



    const int maxAudioPlayerNum = 10;
   
    std::vector<std::shared_ptr<AYAudioPlayer>> _players;
    std::vector<ActiveAudioSource> _activeSources;
    std::unordered_map<std::string, std::weak_ptr<IAYAudioSource>> _audioCache;
    std::mutex _cacheMutex; // 专门用于缓存访问
    std::mutex _sourceMutex; // 专门用于活动源管理
    float _masterVolume = 1.0f;
    glm::vec3 _listenerPosition = glm::vec3(0);
    glm::vec3 _listenerForward = glm::vec3(0);
    glm::vec3 _listenerUp = glm::vec3(0);
    bool _initialized = false;
    float _rolloffFactor = 1.f;
    float _referenceDistance = 20.f;
    float _maxDistance = 500.f;

    std::vector<std::unique_ptr<AYEventToken>> _tokens;

    AYConfigWrapper _preloadFile;
    std::string _preloadPath;
};

REGISTER_MODULE_CLASS("SoundEngine", AYSoundEngine)