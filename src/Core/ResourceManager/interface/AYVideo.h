#pragma once
#include "AYAudioStream.h"

#include <opencv2/opencv.hpp>

class AYVideo : public IAYResource
{
    SUPPORT_MEMORY_POOL(AYVideo)
    DECLARE_RESOURCE_CLASS(AYVideo, "videos")
public:
    using SyncCallback = std::function<void(AYVideo&)>;

    AYVideo();

    virtual ~AYVideo();

    // 获取视频信息
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    float getFPS() const { return _fps; }
    float getFrameInterval() const {return 1.0f / _fps; }
    float getDuration() const { return _duration; }
    bool hasAudio() const { return _audio != nullptr; }
    std::shared_ptr<AYAudioStream> getAudio() { return _audio; }
    const cv::Mat& getCurrentFrameData() const;
    const uint8_t* getCurrentFramePixelData() const;

    // 状态管理
    bool isPlaying() const { return _isPlaying; }
    bool isEndOfVideo() const { return _isEndOfVideo; }
    void play() { _isPlaying = true; }
    void pause() {_isPlaying = false; }
    void stop();

    // 帧控制
    void updateFrame(float delta_time);
    bool advanceFrame();
    bool seekToTime(float seconds); 
    bool seekToFrame(int frameIndex); // 基于帧索引的seek
    void setSyncCallback(SyncCallback callback);
    void syncToAudio(double sync_time);

    // 资源管理
    virtual bool load(const std::string& filepath) override;   
    virtual bool unload() override;
    virtual bool reload(const std::string& filepath) override;
    virtual size_t sizeInBytes() override;

private:
    void releaseData();
    bool decodeNextFrame();
    bool seekToTimestamp(int64_t timestamp);
    bool convertFrameToMat(AVFrame* frame);

    // 视频基础信息
    std::shared_ptr<AYAudioStream> _audio;
    int _width = 0;
    int _height = 0;
    float _fps = 0.0f;
    float _duration = 0.0f;
    int _totalFrames = 0;

    // 音视频同步

    SyncCallback _syncCallback;

    // 解码与帧管理
    AVFormatContext* _formatContext = nullptr;
    AVCodecContext* _codecContext = nullptr;
    int _videoStreamIndex = -1;
    int _audioStreamIndex = -1;

    // 当前帧索引 & 状态
    cv::Mat _currentFrame; // 最后解码的帧
    int64_t _currentPts = AV_NOPTS_VALUE;
    int _currentFrameIndex = 0;
    bool _isPlaying = false;
    bool _isEndOfVideo = false;

    mutable std::recursive_mutex  _dataMutex;

};

REGISTER_RESOURCE_CLASS(AYVideo, 0)