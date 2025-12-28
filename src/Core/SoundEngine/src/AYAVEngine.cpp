#include "AYAVEngine.h"
#include "AYPath.h"
#include "Event_CameraMove.h"
#include "AYLogger.h"
namespace ayt::engine::resource
{
    using namespace ::ayt::engine::config;

    AYAVEngine::AYAVEngine() :
        _preloadPath("@audios/AudioCachePreload.json")
    {
        ALCdevice* device = alcOpenDevice(nullptr);
        if (!device) {
            AYLOG_ERR("[AYAVEngine] Failed to open OpenAL device");
            return;
        }

        ALCcontext* context = alcCreateContext(device, nullptr);
        if (!context) {
            AYLOG_ERR("[AYAVEngine] Failed to create OpenAL context");
            alcCloseDevice(device);
            return;
        }

        if (!alcMakeContextCurrent(context)) {
            AYLOG_ERR("[AYAVEngine] Failed to make OpenAL context current");
            alcDestroyContext(context);
            alcCloseDevice(device);
            return;
        }

        _analyzer = std::make_unique<AYSpectrumAnalyzer>();
        for (int i = 0; i < maxAudioPlayerNum; i++)
        {
            _players.push_back(std::make_shared<AYAudioPlayer>());
            _players.back()->set3DParameters(_rolloffFactor, _referenceDistance, _maxDistance);
        }

    }

    AYAVEngine::~AYAVEngine()
    {
        ALCcontext* context = alcGetCurrentContext();
        ALCdevice* device = alcGetContextsDevice(context);
        alcMakeContextCurrent(nullptr);
        if (context) alcDestroyContext(context);
        if (device) alcCloseDevice(device);
    }

    void AYAVEngine::init()
    {
        if (_initialized) return;

        auto system = GET_CAST_MODULE(EventSystem, "EventSystem");
        if (system)
        {
            auto token = system->subscribe(Event_CameraMove::staticGetType(),
                [this](const IEvent& in_event) {
                    auto& event = static_cast<const Event_CameraMove&>(in_event);
                    auto& trans = event.transform;
                    auto& tp = trans.position;
                    //AYLOG_INFO("[AYAVEngine] update sound location[{},{},{}]", tp.x, tp.y, tp.z);
                    setListenerPosition(trans.position, trans.getForwardVector(), trans.getUpVector());
                });
            _tokens.push_back(std::unique_ptr<EventToken>(token));
        }
        //_preloadAudios();
        _initialized = true;
    }

    void AYAVEngine::update(float delta_time)
    {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);

        // 更新监听器位置
        alListener3f(AL_POSITION, _listenerPosition.x, _listenerPosition.y, _listenerPosition.z);

        float orientation[6] = {
            _listenerForward.x, _listenerForward.y, _listenerForward.z,
            _listenerUp.x, _listenerUp.y, _listenerUp.z
        };
        alListenerfv(AL_ORIENTATION, orientation);

        // 更新音频逻辑
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

        // 频谱分析（针对背景音乐）
        if (_players[AUDIO_SLOT_01]->getState() == AYAudioPlayer::PlayState::Playing) {
            auto bgm = _players[AUDIO_SLOT_01]->getAudioSource();
            if (bgm && bgm->isStreaming()) {
                auto stream = std::static_pointer_cast<AYAudioStream>(bgm);
                auto samples = stream->getCurrentPCM(_analyzer->getFFTSize());
                _analyzer->analyze(samples);
                _currentSpectrum = _analyzer->getBandEnergies(32); // 32频段
            }
        }

        // 更新视频逻辑
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

    void AYAVEngine::shutdown()
    {
        stopAll();

        //_saveAudios();

        _audioCache.clear();

        _tokens.clear();

        _initialized = false;
        AYLOG_INFO("[AYAVEngine] shutdown complete");
    }


    std::shared_ptr<IAYAudioSource> AYAVEngine::playSound2D(const std::string& path, bool isStreaming, bool loop, float volume, int slot, bool asyncload)
    {
        return playSound3D(path, glm::vec3(0, 0, 0), isStreaming, loop, volume, slot, asyncload);
    }

    std::shared_ptr<IAYAudioSource> AYAVEngine::playSound3D(const std::string& path, const glm::vec3& position, bool isStreaming, bool loop, float volume, int slot, bool asyncload)
    {
        if (!_initialized) return nullptr;

        std::shared_ptr<IAYAudioSource> audio;

        // 检查缓存
        {
            std::lock_guard<std::recursive_mutex> cacheLock(_cacheMutex);
            auto it = _audioCache.find(path);
            if (it != _audioCache.end()) {
                audio = it->second.lock();
                if (audio) {
                    // 缓存命中，直接播放
                    _playAudioImpl(audio, position, loop, volume, position != glm::vec3(0), slot);
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
                        [this, position, loop, volume, path, slot](std::shared_ptr<AYAudioStream> loadedAudio) {
                            if (loadedAudio) {
                                std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
                                _audioCache[path] = loadedAudio;
                                _playAudioImpl(loadedAudio, position, loop, volume, position != glm::vec3(0, 0, 0), slot);
                            }
                        });
                }
                else
                {
                    rm.loadAsync<AYAudio>(path,
                        [this, position, loop, volume, path, slot](std::shared_ptr<AYAudio> loadedAudio) {
                            if (loadedAudio) {
                                std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
                                _audioCache[path] = loadedAudio;
                                _playAudioImpl(loadedAudio, position, loop, volume, position != glm::vec3(0, 0, 0), slot);
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

                std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
                _audioCache[path] = audio;
            }
        }

        _playAudioImpl(audio, position, loop, volume, position != glm::vec3(0, 0, 0), slot);
        return audio;
    }

    std::shared_ptr<AYVideo> AYAVEngine::playVideo(const std::string& path, bool loop, const std::shared_ptr<IAYAudioSource>& audio)
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

    void AYAVEngine::createPlaylist(const std::string& playlistName, const std::vector<std::string>& audioPaths) {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        _playlists[playlistName] = { audioPaths, false, 0, nullptr };
    }

    void AYAVEngine::removePlaylist(const std::string& playlistName) {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        if (_currentPlaylistName == playlistName) {
            stopPlaylist();
        }
        _playlists.erase(playlistName);
    }

    void AYAVEngine::playPlaylist(const std::string& playlistName, bool loopPlaylist) {

        auto it = _playlists.find(playlistName);
        if (it == _playlists.end()) {
            spdlog::warn("[AYAVEngine] Playlist '{}' not found", playlistName);
            return;
        }

        _currentPlaylistName = playlistName;
        _playlistLoop = loopPlaylist;
        _isPlaylistPlaying = true;

        auto& playlist = it->second;
        playlist.loop = loopPlaylist;
        playlist.currentTrackIndex = -1;

        // 播放第一首曲目
        _playNextTrack();

    }

    void AYAVEngine::pausePlaylist() {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        if (!_isPlaylistPlaying) return;

        auto it = _playlists.find(_currentPlaylistName);
        if (it != _playlists.end() && it->second.currentAudio) {
            if (auto player = _players[AUDIO_SLOT_01]) {
                player->pause();
            }
        }
        _isPlaylistPlaying = false;
    }

    void AYAVEngine::resumePlaylist() {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        if (_isPlaylistPlaying) return;

        auto it = _playlists.find(_currentPlaylistName);
        if (it != _playlists.end() && it->second.currentAudio) {
            if (auto player = _players[AUDIO_SLOT_01]) {
                player->resume();
            }
        }
        _isPlaylistPlaying = true;
    }

    void AYAVEngine::stopPlaylist() {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        if (!_isPlaylistPlaying) return;

        auto it = _playlists.find(_currentPlaylistName);
        if (it != _playlists.end() && it->second.currentAudio) {
            if (auto player = _players[AUDIO_SLOT_01]) {
                player->stop();
            }
            it->second.currentAudio.reset();
        }
        _isPlaylistPlaying = false;
        _currentPlaylistName.clear();
    }

    void AYAVEngine::skipToNextTrack() {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        if (!_isPlaylistPlaying) return;
        _playNextTrack();
    }

    void AYAVEngine::skipToPreviousTrack() {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        if (!_isPlaylistPlaying) return;
        _playPreviousTrack();
    }

    void AYAVEngine::setPlaylistLoop(bool loop) {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        _playlistLoop = loop;

        auto it = _playlists.find(_currentPlaylistName);
        if (it != _playlists.end()) {
            it->second.loop = loop;
        }
    }

    bool AYAVEngine::isPlaylistPlaying() const {
        return _isPlaylistPlaying;
    }

    std::string AYAVEngine::getCurrentTrack() const {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        auto it = _playlists.find(_currentPlaylistName);
        if (it != _playlists.end() && it->second.currentTrackIndex < it->second.tracks.size()) {
            return it->second.tracks[it->second.currentTrackIndex];
        }
        return "";
    }

    size_t AYAVEngine::getCurrentTrackIndex() const {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        auto it = _playlists.find(_currentPlaylistName);
        if (it != _playlists.end()) {
            return it->second.currentTrackIndex;
        }
        return 0;
    }

    const std::vector<std::string>& AYAVEngine::getPlaylistTracks(const std::string& playlistName) const {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        static const std::vector<std::string> empty;
        auto it = _playlists.find(playlistName);
        if (it != _playlists.end()) {
            return it->second.tracks;
        }
        return empty;
    }

    void AYAVEngine::_playNextTrack() {
        auto it = _playlists.find(_currentPlaylistName);
        if (it == _playlists.end()) return;

        auto& playlist = it->second;

        // 停止当前曲目
        if (playlist.currentAudio) {
            if (auto player = _players[AUDIO_SLOT_01]) {
                player->stop();
            }
            playlist.currentAudio.reset();
        }

        size_t attempts = 0;
        const size_t maxAttempts = playlist.tracks.size();

        while (attempts++ < maxAttempts) {
            // 检查是否到达列表末尾
            if (playlist.currentTrackIndex + 1 >= playlist.tracks.size()) {
                if (playlist.loop) {
                    playlist.currentTrackIndex = 0;
                }
                else {
                    _isPlaylistPlaying = false;
                    _currentPlaylistName.clear();
                    return;
                }
            }
            else {
                playlist.currentTrackIndex++;
            }

            // 尝试播放下一首曲目
            playlist.currentAudio = playSound2D(playlist.tracks[playlist.currentTrackIndex], true, false, 1.0f, AUDIO_SLOT_01, false);

            // 如果播放成功，设置回调并退出循环
            if (playlist.currentAudio) {
                if (auto player = _players[AUDIO_SLOT_01]) {
                    player->setPlaybackFinishedCallback([this]() {
                        _onTrackFinished();
                        });
                }
                break;
            }

            spdlog::warn("[AYAVEngine] Failed to play track: {}", playlist.tracks[playlist.currentTrackIndex]);
        }

        // 如果所有尝试都失败了，停止播放列表
        if (!playlist.currentAudio && attempts >= maxAttempts) {
            AYLOG_ERR("[AYAVEngine] All tracks in playlist '{}' failed to play", _currentPlaylistName);
            _isPlaylistPlaying = false;
            _currentPlaylistName.clear();
        }
    }

    void AYAVEngine::_playPreviousTrack() {
        auto it = _playlists.find(_currentPlaylistName);
        if (it == _playlists.end()) return;

        auto& playlist = it->second;

        // 停止当前曲目
        if (playlist.currentAudio) {
            if (auto player = _players[AUDIO_SLOT_01]) {
                player->stop();
            }
            playlist.currentAudio.reset();
        }

        size_t attempts = 0;
        const size_t maxAttempts = playlist.tracks.size();

        while (attempts++ < maxAttempts) {
            // 检查是否到达列表开头
            if (playlist.currentTrackIndex == 0) {
                if (playlist.loop) {
                    playlist.currentTrackIndex = playlist.tracks.size() - 1;
                }
                else {
                    return;
                }
            }
            else {
                playlist.currentTrackIndex--;
            }

            // 播放上一首曲目
            playlist.currentAudio = playSound2D(playlist.tracks[playlist.currentTrackIndex], true, false, 1.0f, AUDIO_SLOT_01, false);

            // 设置播放完成回调
            if (playlist.currentAudio) {
                if (auto player = _players[AUDIO_SLOT_01]) {
                    player->setPlaybackFinishedCallback([this]() {
                        _onTrackFinished();
                        });
                }
                break;
            }
            spdlog::warn("[AYAVEngine] Failed to play track: {}", playlist.tracks[playlist.currentTrackIndex]);
        }

        // 如果所有尝试都失败了，停止播放列表
        if (!playlist.currentAudio && attempts >= maxAttempts) {
            AYLOG_ERR("[AYAVEngine] All tracks in playlist '{}' failed to play", _currentPlaylistName);
            _isPlaylistPlaying = false;
            _currentPlaylistName.clear();
        }
    }

    void AYAVEngine::_onTrackFinished()
    {
        AYLOG_INFO("[AYAVEngine] one music finished");
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        if (!_isPlaylistPlaying) return;
        _playNextTrack();
    }


    void AYAVEngine::setMasterVolume(float volume)
    {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        float _oldMasterInv = 1 / _masterVolume;
        _masterVolume = std::clamp(volume, 0.0f, 1.0f);

        for (auto& player : _players) {
            player->setMasterVolume(volume);
        }
    }

    void AYAVEngine::pauseAll()
    {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        for (auto& source : _activeAudios) {
            source.player->pause();
        }
        for (auto& video : _activeVideos) {
            video->pause();
        }
    }

    void AYAVEngine::resumeAll()
    {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        for (auto& source : _activeAudios) {
            source.player->resume();
        }
        for (auto& video : _activeVideos) {
            video->play();
        }
    }

    void AYAVEngine::stopAll()
    {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        for (auto& source : _activeAudios) {
            source.player->stop();
        }
        for (auto& video : _activeVideos) {
            video->stop();
        }
        _activeAudios.clear();
        _activeVideos.clear();
    }

    void AYAVEngine::seek(const std::string& path, float seconds)
    {
        // 查找对应的音频和视频资源
        std::shared_ptr<IAYAudioSource> audio;
        std::shared_ptr<AYVideo> video;

        {
            std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
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

    void AYAVEngine::setSourcePosition(std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& position)
    {
        std::lock_guard<std::mutex> lock(_sourceMutex);

        // 查找音源
        auto it = std::find_if(_activeAudios.begin(), _activeAudios.end(),
            [audio](const ActiveAudioSource& s) { return s.audio.lock() == audio; });

        if (it != _activeAudios.end()) {
            it->player->setPosition(position);
        }
    }

    void AYAVEngine::setSourceVelocity(std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& velocity)
    {
        std::lock_guard<std::mutex> lock(_sourceMutex);

        auto it = std::find_if(_activeAudios.begin(), _activeAudios.end(),
            [audio](const ActiveAudioSource& s) { return s.audio.lock() == audio; });

        if (it != _activeAudios.end()) {
            it->player->setVelocity(velocity);
        }
    }

    void AYAVEngine::setAttenuationParameter(float rolloff_factor, float reference_distance, float max_distance)
    {
        _rolloffFactor = rolloff_factor;
        _referenceDistance = reference_distance;
        _maxDistance = max_distance;

        for (auto player : _players)
            player->set3DParameters(_rolloffFactor, _referenceDistance, _maxDistance);
    }

    void AYAVEngine::setListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
    {
        std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
        _listenerPosition = position;
        _listenerForward = glm::normalize(forward);
        _listenerUp = glm::normalize(up);
    }

    void AYAVEngine::_preloadAudios()
    {
        _preloadFile.loadFromFile(_preloadPath, ConfigType::JSON);

        auto& rm = AYResourceManager::getInstance();
        for (const auto& path : _preloadFile.getVector<std::string>("audio_preload"))
        {
            rm.loadAsync<AYAudio>(path, [this, path](std::shared_ptr<AYAudio> loadedAudio) {
                std::lock_guard<std::recursive_mutex> lock(_cacheMutex);
                _audioCache[path] = loadedAudio;
                });
        }
    }

    void AYAVEngine::_saveAudios()
    {
        std::vector<std::string> paths;
        for (const auto& [path, audio] : _audioCache)
        {
            if (!audio.lock()->isStreaming())
                paths.push_back(path);
        }
        _preloadFile.set<std::string>("audio_preload", paths);
        _preloadFile.saveConfig(_preloadPath);
    }

    void AYAVEngine::_playAudioImpl(const std::shared_ptr<IAYAudioSource>& audio, const glm::vec3& position, bool loop, float volume, bool is3D, int slot)
    {
        std::shared_ptr<AYAudioPlayer> splayer;

        if (slot == -1)
        {
            // 查找非背景音乐图层
            int index = 0;
            for (auto player : _players)
            {
                if (player->isAvaliableOrInterruptible() && index)
                {
                    splayer = player;
                    break;
                }
                index++;
            }
        }
        else
        {
            if (slot >= maxAudioPlayerNum)
            {
                AYLOG_ERR("[AYAVEngine] Slot unavailable!");
                return;
            }

            size_t cacheSize = _players.size();
            if (slot >= cacheSize)
            {
                _players.resize(slot + 1);
                for (int i = cacheSize; i < _players.size(); i++)
                {
                    _players[i]->set3DParameters(_rolloffFactor, _referenceDistance, _maxDistance);
                }
            }
            splayer = _players[slot];
        }


        if (!splayer)
            return;

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

    void AYAVEngine::_syncVideoToAudio(const std::shared_ptr<AYVideo>& video, const std::shared_ptr<IAYAudioSource>& audio)
    {
        video->setSyncCallback([audio](AYVideo& v) {
            if (auto player = dynamic_cast<AYAudioPlayer*>(audio.get())) {
                double audioTime = player->getCurrentTime();
                v.syncToAudio(audioTime);
            }
            });
    }
}