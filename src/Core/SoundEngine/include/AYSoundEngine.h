#pragma once
#include "Mod_SoundEngine.h"
#include "AYResourceManager.h"
#include "AYAudio.h"
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
    ~AYSoundEngine() = default;

    void init() override;
    void shutdown() override;
    void update(float delta_time) override;

    // 音频播放控制
    std::shared_ptr<AYAudio> play2D(const std::string& path,
        bool loop = false,
        float volume = 1.0f,
        bool asyncLoad = true);

    std::shared_ptr<AYAudio> play3D(const std::string& path,
        const glm::vec3& position,
        bool loop = false,
        float volume = 1.0f,
        bool asyncLoad = true);

    // 全局控制
    void setMasterVolume(float volume);
    void pauseAll();
    void resumeAll();
    void stopAll();

    // 音频源管理
    void setListenerPosition(const glm::vec3& position,
        const glm::vec3& forward = glm::vec3(0, 0, -1),
        const glm::vec3& up = glm::vec3(0, 1, 0));

private:
    struct ActiveAudioSource {
        std::weak_ptr<AYAudio> audio;
        ALuint sourceId;
        glm::vec3 position; // 3D音效位置
        bool is3D;
    };  

    void _preloadAudios();
    void _saveAudios();

    void _playAudioImpl(const std::shared_ptr<AYAudio>& audio,
        const glm::vec3& position,
        bool loop,
        float volume,
        bool is3D);

    std::vector<ActiveAudioSource> _activeSources;
    std::unordered_map<std::string, std::weak_ptr<AYAudio>> _audioCache;
    std::mutex _mutex;
    float _masterVolume = 1.0f;
    glm::vec3 _listenerPosition;
    glm::vec3 _listenerForward;
    glm::vec3 _listenerUp;
    bool _initialized = false;

    AYConfigWrapper _preloadFile;
    std::string _preloadPath;
};

REGISTER_MODULE_CLASS("SoundEngine", AYSoundEngine)