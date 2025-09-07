#pragma once
#include "IAYResource.h"
#include "AYResourceRegistry.h"
#include "AYMemoryPool.h"
#include "IAYAudioSource.h"

class AYAudioStream : public IAYResource, public IAYAudioSource
{
    SUPPORT_MEMORY_POOL(AYAudioStream)
    DECLARE_RESOURCE_CLASS(AYAudioStream, "audiostream")
public:
    AYAudioStream();

    virtual ~AYAudioStream();

    bool open(const std::string& path);
    AudioFramePtr decodeNextFrame();

    const std::vector<uint8_t>& getPCMData() const override { return std::vector<uint8_t>(); }
    bool isStreaming() const override { return true; }
    // 获取音频信息
    int getSampleRate() const override { return _sampleRate; }
    int getChannels() const override { return _channels; }
    float getDuration() const override { return _duration; }
    float getSuggestedGain() const override { return 1.f; }
    ALenum getFormat() const override { return _format; }
    double getCurrentPts() const; // 获取当前音频帧的PTS
    bool isSeekable() const;      // 判断是否支持跳转
    double getStartTime() const;   // 获取音频流的开始时间
    bool seekToTime(double seconds);

    virtual bool load(const std::string& filepath) override;
    virtual bool unload() override;
    virtual bool reload(const std::string& filepath) override;
    virtual size_t sizeInBytes() override;

private:
    void releaseData();
    bool initDecoder();
    bool initResampler();

    // FFmpeg 资源
    AVFormatCtxPtr _fmtCtx;
    AVCodecCtxPtr _codecCtx;
    AVSwrCtxPtr _swrCtx;
    int _streamIndex = -1;

    // 音频参数
    int _sampleRate;
    int _channels;
    float _duration;
    ALenum _format;

    // 状态控制
    std::atomic<bool> _isPlaying{ false };
    std::atomic<double> _currentPts{ 0.0 }; // 当前音频PTS（秒）
    std::atomic<double> _startTime{ 0.0 }; // 流开始时间（秒）
    std::atomic<bool> _seekable{ false };   // 是否支持跳转
    std::mutex _decodeMutex;
    std::queue<AudioFramePtr> _frameQueue;
};

REGISTER_RESOURCE_CLASS(AYAudioStream, 0)