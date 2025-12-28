#pragma once
#include "IAYResource.h"
#include "AYResourceRegistry.h"
#include "AYMemoryPool.h"
#include "IAYAudioSource.h"
namespace ayt::engine::resource
{
    class AYAudio : public IResource, public IAYAudioSource
    {
        SUPPORT_MEMORY_POOL(AYAudio)
            DECLARE_RESOURCE_CLASS(AYAudio, "audios")
    public:
        AYAudio();

        virtual ~AYAudio();

        // 音频数据接口
        const std::vector<uint8_t>& getPCMData() const override { return _pcmData; }
        bool isStreaming() const override { return false; } // 静态音频与流式音频

        // 获取音频信息
        int getSampleRate() const override { return _sampleRate; }
        int getChannels() const override { return _channels; }
        float getDuration() const override { return _duration; }
        float getSuggestedGain() const override { return 1.f; }
        ALenum getFormat() const override { return _format; }

        virtual bool load(const std::string& filepath) override;

        virtual bool unload() override;

        virtual bool reload(const std::string& filepath) override;

        virtual size_t sizeInBytes() override;

    private:
        bool decodeFullAudio(const std::string& path);
        void releaseData();

        std::vector<uint8_t> _pcmData;  // 原始数据
        int _sampleRate;
        int _channels;
        float _duration;
        ALenum _format;
        mutable std::mutex _dataMutex;
    };

    REGISTER_RESOURCE_CLASS(AYAudio, 0)
}