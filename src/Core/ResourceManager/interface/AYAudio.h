#pragma once
#include "IAYResource.h"
#include "AYResourceRegistry.h"
#include "AYMemoryPool.h"
#include <iostream>
#include <atomic>
#include <AL/al.h>
#include <AL/alc.h>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h> 
}

class AYAudio : public IAYResource
{
    SUPPORT_MEMORY_POOL(AYAudio)
    DECLARE_RESOURCE_CLASS(AYAudio, "audios")
public:
    AYAudio() :
        _audioBuffer(0),
        _sampleRate(0),
        _channels(0),
        _format(AL_FORMAT_MONO16), // 默认单声道16位
        _duration(0.0f)
    {
    }

    virtual ~AYAudio() {
        releaseData();
    }

    bool playAsync(bool loop = false) {
        if (!_audioBuffer) {
            std::cerr << "Play failed: No audio buffer loaded" << std::endl;
            return false;
        }

        ALint bufferSize, channels, sampleRate;
        alGetBufferi(_audioBuffer, AL_SIZE, &bufferSize);
        //alGetBufferi(_audioBuffer, AL_CHANNELS, &channels);
        //alGetBufferi(_audioBuffer, AL_FREQUENCY, &sampleRate);

        //std::cout << "Buffer info - Size: " << bufferSize
        //    << ", Channels: " << channels
        //    << ", Sample Rate: " << sampleRate << std::endl;

        if (bufferSize <= 0) {
            std::cerr << "Error: Loaded empty audio buffer" << std::endl;
            return false;
        }

        // 检查OpenAL上下文
        ALCcontext* context = alcGetCurrentContext();
        if (!context) {
            std::cerr << "Play failed: No OpenAL context available" << std::endl;
            return false;
        }

        // 检查缓冲区有效性
        if (!alIsBuffer(_audioBuffer)) {
            std::cerr << "Play failed: Invalid audio buffer" << std::endl;
            return false;
        }

        // 创建或重用音频源
        if (!_source) {
            alGenSources(1, &_source);
            ALenum error = alGetError();
            if (error != AL_NO_ERROR) {
                std::cerr << "Play failed: Could not generate source (error: " << error << ")" << std::endl;
                return false;
            }

            alSourcei(_source, AL_BUFFER, _audioBuffer);
            error = alGetError();
            if (error != AL_NO_ERROR) {
                std::cerr << "Play failed: Could not bind buffer to source (error: " << error << ")" << std::endl;
                return false;
            }
        }

        // 设置源参数
        alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcef(_source, AL_GAIN, 1.0f); // 确保音量不是0

        // 尝试播放
        alSourcePlay(_source);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cerr << "Play failed: alSourcePlay failed (error: " << error << ")" << std::endl;
            return false;
        }

        // 验证播放状态
        ALint state;
        alGetSourcei(_source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            std::cerr << "Play failed: Source in unexpected state: " << state << std::endl;
            return false;
        }

        return true;
    }

    void pause() {
        if (_source) {
            alSourcePause(_source);
        }
    }

    void resume() {
        if (_source) {
            alSourcePlay(_source);
        }
    }

    void stop() {
        if (_source) {
            alSourceStop(_source);
            alDeleteSources(1, &_source);
            _source = 0;
        }
    }

    // 获取当前播放状态
    bool isPlaying() const {
        if (!_source) return false;
        ALint state;
        alGetSourcei(_source, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING;
    }

    bool isPaused() const {
        if (!_source) return false;
        ALint state;
        alGetSourcei(_source, AL_SOURCE_STATE, &state);
        return state == AL_PAUSED;
    }

    // 获取OpenAL音频缓冲区ID
    ALuint getAudioBuffer() const { return _audioBuffer; }

    // 获取音频信息
    int getSampleRate() const { return _sampleRate; }
    int getChannels() const { return _channels; }
    float getDuration() const { return _duration; }

    virtual bool load(const std::string& filepath) override
    {
        if (_loaded) return true;

        // 初始化FFmpeg
        AVFormatContext* formatContext = nullptr;
        // 打开文件
        if (avformat_open_input(&formatContext, filepath.c_str(), nullptr, nullptr) != 0) {
            std::cerr << "Failed to open input file: " << filepath << std::endl;
            return false;
        }

        auto format_deleter = [](AVFormatContext* ctx) {
            if (ctx) avformat_close_input(&ctx);
            };
        std::unique_ptr<AVFormatContext, decltype(format_deleter)>
            formatGuard(formatContext, format_deleter);

        // 获取流信息
        if (avformat_find_stream_info(formatContext, nullptr) < 0) {
            std::cerr << "Failed to find stream info" << std::endl;
            return false;
        }

        // 查找音频流，遍历所有流，找到AVMEDIA_TYPE_AUDIO类型，获取 编解码参数
        int audioStreamIndex = -1;
        AVCodecParameters* codecParameters = nullptr;
        for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
            if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioStreamIndex = i;
                codecParameters = formatContext->streams[i]->codecpar;
                break;
            }
        }

        if (audioStreamIndex == -1) {
            return false;
        }

        // 获取音频原生参数
        _sampleRate = codecParameters->sample_rate;
        _channels = codecParameters->ch_layout.nb_channels;

        // 确定OpenAL格式
        if (_channels == 1) {
            _format = AL_FORMAT_MONO16;
        }
        else if (_channels == 2) {
            _format = AL_FORMAT_STEREO16;
        }
        else {
            // 对于多声道音频，降级为立体声
            std::cerr << "Unsupported channel count: " << _channels << ", falling back to stereo" << std::endl;
            _format = AL_FORMAT_STEREO16;
            _channels = 2;
        }

        // 计算持续时间
        if (formatContext->duration != AV_NOPTS_VALUE) {
            _duration = static_cast<float>(formatContext->duration) / AV_TIME_BASE;
        }

        // 找到音频的指定解码器
        const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            std::cerr << "Unsupported codec: " << avcodec_get_name(codecParameters->codec_id) << std::endl;
            return false;
        }

        // 初始化解码器上下文
        auto codec_deleter = [](AVCodecContext* c) { avcodec_free_context(&c); };
        std::unique_ptr<AVCodecContext, decltype(codec_deleter)> codecGuard(
            avcodec_alloc_context3(codec), codec_deleter);

        if (avcodec_parameters_to_context(codecGuard.get(), codecParameters) < 0) {
            std::cerr << "Failed to copy codec parameters" << std::endl;
            return false;
        }

        // 尝试打开解码器
        if (avcodec_open2(codecGuard.get(), codec, nullptr) < 0) {
            std::cerr << "Failed to open codec" << std::endl;
            return false;
        }

        // 初始化重采样上下文
        auto contex_deleter = [](SwrContext* s) { swr_free(&s); };
        std::unique_ptr<SwrContext, decltype(contex_deleter)> swrGuard(swr_alloc(), contex_deleter);

        // 设置重采样参数
        av_opt_set_chlayout(swrGuard.get(), "in_chlayout", &codecGuard.get()->ch_layout, 0);
        av_opt_set_int(swrGuard.get(), "in_sample_rate", codecGuard.get()->sample_rate, 0);
        av_opt_set_sample_fmt(swrGuard.get(), "in_sample_fmt", codecGuard.get()->sample_fmt, 0);

        AVChannelLayout out_chlayout;
        if (_channels == 1) {
            out_chlayout = AV_CHANNEL_LAYOUT_MONO;
        }
        else {
            out_chlayout = AV_CHANNEL_LAYOUT_STEREO;
            // 确保输入的通道数匹配
            if (codecGuard.get()->ch_layout.nb_channels > 2) {
                std::cerr << "Warning: Downmixing multi-channel audio to stereo" << std::endl;
            }
        }

        av_opt_set_chlayout(swrGuard.get(), "out_chlayout", &out_chlayout, 0);
        av_opt_set_int(swrGuard.get(), "out_sample_rate", _sampleRate, 0);
        av_opt_set_sample_fmt(swrGuard.get(), "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

        if (swr_init(swrGuard.get()) < 0) {
            std::cerr << "Failed to initialize resampler" << std::endl;
            return false;
        }

        //std::cout << "Resampler configuration:\n"
        //    << "  Input: " << av_get_sample_fmt_name(codecGuard.get()->sample_fmt)
        //    << ", " << codecGuard.get()->sample_rate << " Hz, "
        //    << codecGuard.get()->ch_layout.nb_channels << " channels\n"
        //    << "  Output: " << av_get_sample_fmt_name(AV_SAMPLE_FMT_S16)
        //    << ", " << _sampleRate << " Hz, "
        //    << out_chlayout.nb_channels << " channels" << std::endl;

        // 读取音频帧并解码
        auto packet_deleter = [](AVPacket* p) { av_packet_free(&p); };
        auto frame_deleter = [](AVFrame* f) { av_frame_free(&f); };
        std::unique_ptr<AVPacket, decltype(packet_deleter)>
            packet(av_packet_alloc(), packet_deleter);

        // 原始帧直接从解码器获取，用于中间处理环节，信息为文件原生
        std::unique_ptr<AVFrame, decltype(frame_deleter)>
            frame(av_frame_alloc(), frame_deleter);

        if (!packet || !frame ) {
            std::cerr << "Failed to allocate frames/packets" << std::endl;
            return false;
        }

        // 重采样帧通过重采样器转换得到，最终可用音频数据，信息为预期信息
        auto resampledFrame = std::shared_ptr<AVFrame>(av_frame_alloc(), frame_deleter);

        std::vector<uint8_t> audioData;
        int totalSamples = 0;

        while (av_read_frame(formatContext, packet.get()) >= 0) {
            if (packet->stream_index == audioStreamIndex) {
                if (avcodec_send_packet(codecGuard.get(), packet.get()) == 0) {
                    while (avcodec_receive_frame(codecGuard.get(), frame.get()) == 0) {
                        // 计算输出样本数
                        int out_samples = av_rescale_rnd(
                            swr_get_delay(swrGuard.get(), frame->sample_rate) + frame->nb_samples,
                            _sampleRate, frame->sample_rate, AV_ROUND_UP);

                        // 配置重采样输出帧
                        resampledFrame->sample_rate = _sampleRate;
                        resampledFrame->format = AV_SAMPLE_FMT_S16;
                        av_channel_layout_copy(&resampledFrame->ch_layout, &out_chlayout);
                        resampledFrame->nb_samples = out_samples;

                        if (av_frame_get_buffer(resampledFrame.get(), 0) < 0) {
                            std::cerr << "Failed to allocate resampled frame buffer" << std::endl;
                            continue;
                        }

                        // 执行重采样
                        int ret = swr_convert(swrGuard.get(),
                            resampledFrame->data, out_samples,
                            (const uint8_t**)frame->data, frame->nb_samples);
                        if (ret < 0) {
                            char errbuf[AV_ERROR_MAX_STRING_SIZE];
                            av_strerror(ret, errbuf, sizeof(errbuf));
                            std::cerr << "swr_convert failed: " << errbuf << std::endl;
                            continue;
                        }

                        // 更新实际样本数
                        resampledFrame->nb_samples = ret;

                        // 处理重采样后的数据
                        int data_size = ret * out_chlayout.nb_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                        audioData.insert(audioData.end(),
                            resampledFrame->data[0],
                            resampledFrame->data[0] + data_size);
                    }
                }
            }
            av_packet_unref(packet.get());
        }

        // 创建OpenAL缓冲区
        alGenBuffers(1, &_audioBuffer);
        alBufferData(_audioBuffer, _format, audioData.data(), static_cast<ALsizei>(audioData.size()), _sampleRate);

        ALenum alError = alGetError();
        if (alError != AL_NO_ERROR) {
            std::cerr << "OpenAL error: " << alError << std::endl;
            releaseData();
            return false;
        }

        IAYResource::load(filepath);
        _loaded = true;
        return true;
    }

    virtual bool unload() override
    {
        if (!_loaded) return true;
        std::lock_guard<std::mutex> lock(_dataMutex);
        releaseData();
        return true;
    }

    virtual bool reload(const std::string& filepath) override
    {
        unload();
        return load(filepath);
    }

    virtual size_t sizeInBytes() override
    {
        // 估算音频数据大小 (样本数 * 通道数 * 2字节)
        return static_cast<size_t>(_duration * _sampleRate * _channels * 2);
    }

private:
    void releaseData() {
        stop();

        if (_audioBuffer && alIsBuffer(_audioBuffer)) {
            alDeleteBuffers(1, &_audioBuffer);
            _audioBuffer = 0;  // 重置为无效值
        }
        // 重置其他成员变量
        _sampleRate = 0;
        _channels = 0;
        _format = AL_FORMAT_MONO16;
        _duration = 0.0f;
    }

    ALuint _audioBuffer;
    ALuint _source = 0;
    int _sampleRate;
    int _channels;
    ALenum _format;
    float _duration;
    mutable std::mutex _dataMutex;
};

REGISTER_RESOURCE_CLASS(AYAudio, 0)