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
    bool seek(double seconds);

    const std::vector<uint8_t>& getPCMData() const override { return std::vector<uint8_t>(); }
    bool isStreaming() const override { return true; }
    // 获取音频信息
    int getSampleRate() const override { return _sampleRate; }
    int getChannels() const override { return _channels; }
    float getDuration() const override { return _duration; }
    ALenum getFormat() const override { return _format; }

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
    std::mutex _decodeMutex;
    std::queue<AudioFramePtr> _frameQueue;
};

REGISTER_RESOURCE_CLASS(AYAudioStream, 0)