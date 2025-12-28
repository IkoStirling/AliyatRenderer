#include "AYAudio.h"

namespace ayt::engine::resource
{
    using ayt::engine::path::Path;

    AYAudio::AYAudio() :
        _sampleRate(0),
        _channels(0),
        _format(AL_FORMAT_MONO16), // 默认单声道16位
        _duration(0.0f)
    {
    }

    AYAudio::~AYAudio()
    {
        releaseData();
    }

    bool AYAudio::load(const std::string& filepath)
    {
        if (_loaded) return true;

        std::string rpath = Path::resolve(filepath);

        // 解码音频并提取 PCM
        if (!decodeFullAudio(rpath)) return false;

        IAYResource::load(rpath);
        _loaded = true;
        return true;
    }

    bool AYAudio::unload()
    {
        if (!_loaded) return true;
        std::lock_guard<std::mutex> lock(_dataMutex);
        releaseData();
        return true;
    }

    bool AYAudio::reload(const std::string& filepath)
    {
        std::string rpath = Path::resolve(filepath);
        unload();
        return load(rpath);
    }

    size_t AYAudio::sizeInBytes()
    {
        // 估算音频数据大小 (样本数 * 通道数 * 16bit)
        return static_cast<size_t>(_duration * _sampleRate * _channels * 2);
    }

    void AYAudio::releaseData()
    {
        // 重置其他成员变量
        _sampleRate = 0;
        _channels = 0;
        _format = AL_FORMAT_MONO16;
        _duration = 0.0f;
    }

    /*
        1、使用 FFmpeg 打开音频文件，查找音频流（AVMEDIA_TYPE_AUDIO）

        2、获取音频流的原始参数：采样率、声道数、编码格式等

        3、根据路径判断是否为环境音效，决定是否转为单声道

        4、找到对应的解码器（比如 MP3 解码器）并初始化解码上下文

        5、设置重采样器（SwrContext），将音频转为统一的格式：​​S16（16bit 整数）、目标采样率、单声道/立体声​​

        6、逐帧解码音频，对每一帧调用 swr_convert()进行重采样，提取 PCM 数据

        7、处理解码器剩余数据（flush）

        8、保存最终的 PCM 数据、采样率、声道数、格式、时长等信息
    */
    bool AYAudio::decodeFullAudio(const std::string& rpath)
    {
        // --------------------------
        // 1. 创建 FormatContext 并查找音频流
        // --------------------------
        auto formatContext = AVCreator::createFormatContext(rpath);

        AVCodecParameters* codecParameters = nullptr;
        int audioStreamIndex = -1;
        audioStreamIndex = formatContext->findBsetStream(codecParameters, AVMediaType::AVMEDIA_TYPE_AUDIO);

        if (audioStreamIndex == -1 || !codecParameters)
            return false;

        // --------------------------
        // 2. 获取音频基本参数（采样率、声道数等）
        // --------------------------
        _sampleRate = codecParameters->sample_rate;
        _channels = codecParameters->ch_layout.nb_channels;

        // 是否为环境音效（决定是否保留多声道）
        bool isAmbient = (rpath.find("/ambient/") != std::string::npos);
        if (!isAmbient)
            _channels = 1;

        // 设置 OpenAL 格式和声道布局
        AVChannelLayout out_chlayout;
        if (!isAmbient) {
            _channels = 1;
            _format = AL_FORMAT_MONO16;
            out_chlayout = AV_CHANNEL_LAYOUT_MONO;
        }
        else {
            _channels = 2;
            _format = AL_FORMAT_STEREO16;
            out_chlayout = AV_CHANNEL_LAYOUT_STEREO;
        }

        // 计算音频时长（秒）
        if (formatContext->get()->duration != AV_NOPTS_VALUE) {
            _duration = static_cast<float>(formatContext->get()->duration) / AV_TIME_BASE;
        }

        // --------------------------
        // 3. 找到音频解码器并初始化解码器上下文
        // --------------------------
        const AVCodec* codec = AVCreator::findDecoder(codecParameters);
        if (!codec)
            return false;

        auto codecContext = AVCreator::createCodecContext(codec);
        if (avcodec_parameters_to_context(codecContext.get(), codecParameters) < 0 ||
            avcodec_open2(codecContext.get(), codec, nullptr) < 0) {
            std::cerr << "Failed to setup decoder" << std::endl;
            return false;
        }


        // --------------------------
        // 4. 初始化重采样上下文（转 S16 / 目标声道布局）
        // --------------------------
        auto swrContext = AVCreator::createSwrContext();

        av_opt_set_chlayout(swrContext.get(), "in_chlayout", &codecContext->ch_layout, 0);
        av_opt_set_int(swrContext.get(), "in_sample_rate", codecContext->sample_rate, 0);
        av_opt_set_sample_fmt(swrContext.get(), "in_sample_fmt", codecContext->sample_fmt, 0);

        int out_sample_rate = codecContext->sample_rate;
        AVChannelLayout  out_channels = codecContext->ch_layout;
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16; // OpenAL通常使用S16格式

        if (out_sample_rate <= 0) out_sample_rate = 44100;
        if (out_channels.nb_channels <= 0) out_channels.nb_channels = 2;

        av_opt_set_chlayout(swrContext.get(), "out_chlayout", &out_channels, 0);
        av_opt_set_int(swrContext.get(), "out_sample_rate", out_sample_rate, 0);
        av_opt_set_sample_fmt(swrContext.get(), "out_sample_fmt", out_sample_fmt, 0);

        if (swr_init(swrContext.get()) < 0) {
            std::cerr << "Failed to initialize resampler" << std::endl;
            return false;
        }

        // --------------------------
        // 5. 解码所有音频帧，并进行重采样 => 提取 PCM 数据
        // --------------------------
        std::vector<uint8_t> pcmData;
        int max_dst_nb_samples = 0;
        uint8_t** dst_data = nullptr;
        int dst_linesize = 0;

        auto operation = [&](AVFrame* origin_frame) -> int {
            if (!origin_frame) return 1;

            // 重采样逻辑（使用 swr_convert）
            int64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swrContext.get(), origin_frame->sample_rate) + origin_frame->nb_samples,
                out_sample_rate, origin_frame->sample_rate, AV_ROUND_UP);

            uint8_t** dst_data = nullptr;
            int dst_linesize = 0;

            if (dst_nb_samples > 0) {
                if (av_samples_alloc_array_and_samples(&dst_data, &dst_linesize,
                    out_channels.nb_channels, (int)dst_nb_samples, out_sample_fmt, 0) < 0) {
                    return -1;
                }

                int nb_samples = swr_convert(swrContext.get(), dst_data, (int)dst_nb_samples,
                    (const uint8_t**)origin_frame->data, origin_frame->nb_samples);

                if (nb_samples > 0) {
                    int bufsize = av_samples_get_buffer_size(&dst_linesize, out_channels.nb_channels,
                        nb_samples, out_sample_fmt, 1);
                    if (bufsize > 0) {
                        pcmData.insert(pcmData.end(), dst_data[0], dst_data[0] + bufsize);
                    }
                }

                av_freep(&dst_data[0]);
                av_freep(&dst_data);
            }

            return 0;
            };

        // 处理所有帧
        AVCreator::runFrameResolvingLoop(formatContext, codecContext.get(), audioStreamIndex, operation);

        // --------------------------
        // 6. 刷新重采样器（取出剩余数据）
        // --------------------------
        int flush_samples = swr_get_delay(swrContext.get(), (int64_t)out_sample_rate);
        if (flush_samples > 0) {
            uint8_t* flush_data = nullptr;
            int flush_linesize = 0;
            int ret = av_samples_alloc(&flush_data, &flush_linesize,
                out_channels.nb_channels, flush_samples, out_sample_fmt, 0);
            if (ret >= 0) {
                int nb_samples = swr_convert(swrContext.get(), &flush_data, flush_samples, nullptr, 0);
                if (nb_samples > 0) {
                    int flush_bufsize = av_samples_get_buffer_size(
                        &flush_linesize, out_channels.nb_channels, nb_samples, out_sample_fmt, 1);
                    pcmData.insert(pcmData.end(), flush_data, flush_data + flush_bufsize);
                }
                av_freep(&flush_data);
            }
        }

        // --------------------------
        // 7. 保存最终 PCM 数据与参数
        // --------------------------
        {
            std::lock_guard<std::mutex> lock(_dataMutex);
            _pcmData = std::move(pcmData);
            _sampleRate = out_sample_rate;
            _channels = out_channels.nb_channels;
            _duration = static_cast<float>(_pcmData.size()) /
                (_sampleRate * _channels * av_get_bytes_per_sample(out_sample_fmt));

            // 设置OpenAL格式
            if (_channels == 1) {
                _format = AL_FORMAT_MONO16;
            }
            else if (_channels == 2) {
                _format = AL_FORMAT_STEREO16;
            }
            else {
                std::cerr << "Unsupported channel count: " << _channels << std::endl;
                return false;
            }
        }

        return true;
    }
}