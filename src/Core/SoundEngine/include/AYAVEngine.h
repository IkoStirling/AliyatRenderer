#pragma once
#include "Mod_AVEngine.h"
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

class AYAVEngine : public Mod_AVEngine
{
public:
    AYAVEngine();
    ~AYAVEngine();

    void init() override;
    void shutdown() override;
    void update(float delta_time) override;

    // 音频播放控制
    const int AUDIO_SLOT_01 = 0;    //背景音频插槽，非指定slot时不使用
    const int AUDIO_SLOT_02 = 1;
    const int AUDIO_SLOT_03 = 2;
    const int AUDIO_SLOT_04 = 3;
    const int AUDIO_SLOT_05 = 4;

    std::shared_ptr<IAYAudioSource> playSound2D(const std::string& path,
        bool isStreaming = false,
        bool loop = false,
        float volume = 1.0f,
        int slot = -1,
        bool asyncload = true);

    std::shared_ptr<IAYAudioSource> playSound3D(const std::string& path,
        const glm::vec3& position,
        bool isStreaming = false,
        bool loop = false,
        float volume = 1.0f,
        int slot = -1,
        bool asyncload = true);

    std::shared_ptr<AYVideo> playVideo(const std::string& path,
        bool loop = false,
        const std::shared_ptr<IAYAudioSource>& audio = nullptr);

    // 音频列表管理
    void createPlaylist(const std::string& playlistName, const std::vector<std::string>& audioPaths);
    void removePlaylist(const std::string& playlistName);
    void playPlaylist(const std::string& playlistName, bool loopPlaylist = false);
    void pausePlaylist();
    void resumePlaylist();
    void stopPlaylist();
    void skipToNextTrack();
    void skipToPreviousTrack();
    void setPlaylistLoop(bool loop);
    bool isPlaylistPlaying() const;
    std::string getCurrentTrack() const;
    size_t getCurrentTrackIndex() const;
    const std::vector<std::string>& getPlaylistTracks(const std::string& playlistName) const;

    // 全局音频控制
    void setMasterVolume(float volume);
    void pauseAll();
    void resumeAll();
    void stopAll();
    void seek(const std::string& path, float seconds);

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
        bool is3D,
        int slot = -1);

    void _syncVideoToAudio(const std::shared_ptr<AYVideo>& video,
        const std::shared_ptr<IAYAudioSource>& audio);

    const int maxAudioPlayerNum = 10;


   
    std::vector<std::shared_ptr<AYAudioPlayer>> _players;
    std::unordered_map<std::string, std::weak_ptr<IAYAudioSource>> _audioCache;
    std::unordered_map<std::string, std::weak_ptr<AYVideo>> _videoCache;
    std::vector<ActiveAudioSource> _activeAudios;
    std::vector<std::shared_ptr<AYVideo>> _activeVideos;

    mutable std::recursive_mutex _cacheMutex; // 专门用于缓存访问
    mutable std::mutex _sourceMutex; // 专门用于活动源管理
    float _masterVolume = 1.0f;
    glm::vec3 _listenerPosition = glm::vec3(0);
    glm::vec3 _listenerForward = glm::vec3(0);
    glm::vec3 _listenerUp = glm::vec3(0);
    bool _initialized = false;
    float _rolloffFactor = 1.f;
    float _referenceDistance = 20.f;
    float _maxDistance = 500.f;

private:
    struct Playlist {
        std::vector<std::string> tracks;
        bool loop = false;
        size_t currentTrackIndex = 0;
        std::shared_ptr<IAYAudioSource> currentAudio;
    };

    std::unordered_map<std::string, Playlist> _playlists;
    std::string _currentPlaylistName;
    bool _isPlaylistPlaying = false;
    bool _playlistLoop = false;

    void _playNextTrack();
    void _playPreviousTrack();
    void _onTrackFinished();

private:
    std::vector<std::unique_ptr<AYEventToken>> _tokens;

    AYConfigWrapper _preloadFile;
    std::string _preloadPath;
};

REGISTER_MODULE_CLASS("AVEngine", AYAVEngine)