#pragma once
#include "AYAudioStream.h"

#include <opencv2/opencv.hpp>

class AYVideo : public IAYResource
{
    SUPPORT_MEMORY_POOL(AYVideo)
    DECLARE_RESOURCE_CLASS(AYVideo, "videos")
public:
    AYVideo();

    virtual ~AYVideo();

    // 获取视频信息
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    float getFPS() const { return _fps; }
    float getDuration() const { return _duration; }
    bool hasAudio() const { return _audio != nullptr; }
    AYAudioStream* getAudio() { return _audio.get(); }
    const cv::Mat& getCurrentFrameData() const { return _imageData; }
    const uint8_t* getCurrentFramePixelData() const;

    // 更新当前帧数据到imageData
    bool updateFrame(float delta_time);

    virtual bool load(const std::string& filepath) override;
    
    virtual bool unload() override;

    virtual bool reload(const std::string& filepath) override;

    virtual size_t sizeInBytes() override;


private:
    void releaseData();

    std::unique_ptr<AYAudioStream> _audio;
    int _width;
    int _height;
    float _fps;
    float _duration;
    int _currentFrame;
    float _accumulatedTime = 0.0f;
    float _frameInterval;
    cv::Mat _imageData;
    AVFormatContext* _formatContext = nullptr;
    AVCodecContext* _codecContext = nullptr;
    int _videoStreamIndex = -1;
    int _audioStreamIndex = -1;
    mutable std::mutex _dataMutex;

};

REGISTER_RESOURCE_CLASS(AYVideo, 0)