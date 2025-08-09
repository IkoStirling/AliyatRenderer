#include "AYAudio.h"

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

    std::string rpath = AYPath::resolve(filepath);

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
    std::string rpath = AYPath::resolve(filepath);
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

bool AYAudio::decodeFullAudio(const std::string& rpath)
{
    auto formatContext = AVCreator::createFormatContext(rpath);

    AVCodecParameters* codecParameters = nullptr;
    int audioStreamIndex = -1;
    audioStreamIndex = formatContext->findStream(codecParameters, AVMediaType::AVMEDIA_TYPE_AUDIO);

    if (audioStreamIndex == -1 || !codecParameters)
        return false;

    // 获取音频原生参数
    _sampleRate = codecParameters->sample_rate;
    _channels = codecParameters->ch_layout.nb_channels;

    // 非环境音效设置为单声道
    bool isAmbient = (rpath.find("/ambient/") != std::string::npos);
    if (!isAmbient)
        _channels = 1;

    AVChannelLayout out_chlayout;
    // 确定OpenAL格式
    if (!isAmbient) {
        _channels = 1;
        _format = AL_FORMAT_STEREO16;
        out_chlayout = AV_CHANNEL_LAYOUT_MONO;
    }
    else {
        _channels = 2;
        _format = AL_FORMAT_MONO16;
        out_chlayout = AV_CHANNEL_LAYOUT_STEREO;
    }

    // 计算持续时间
    if (formatContext->get()->duration != AV_NOPTS_VALUE) {
        _duration = static_cast<float>(formatContext->get()->duration) / AV_TIME_BASE;
    }

    // 找到音频的指定解码器
    const AVCodec* codec = AVCreator::findDecoder(codecParameters);
    if (!codec)
        return false;

    // 初始化解码器上下文
    auto codecContext = AVCreator::createCodecContext(codec);

    if (avcodec_parameters_to_context(codecContext.get(), codecParameters) < 0) {
        std::cerr << "Failed to copy codec parameters" << std::endl;
        return false;
    }

    // 尝试打开解码器
    if (avcodec_open2(codecContext.get(), codec, nullptr) < 0) {
        std::cerr << "Failed to open codec" << std::endl;
        return false;
    }

    // 初始化重采样上下文
    auto swrContext = AVCreator::createSwrContext();
    
    // 设置重采样参数
    av_opt_set_chlayout(swrContext.get(), "in_chlayout", &codecContext->ch_layout, 0);
    av_opt_set_int(swrContext.get(), "in_sample_rate", codecContext->sample_rate, 0);
    av_opt_set_sample_fmt(swrContext.get(), "in_sample_fmt", codecContext->sample_fmt, 0);

    av_opt_set_chlayout(swrContext.get(), "out_chlayout", &out_chlayout, 0);
    av_opt_set_int(swrContext.get(), "out_sample_rate", _sampleRate, 0);
    av_opt_set_sample_fmt(swrContext.get(), "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    if (swr_init(swrContext.get()) < 0) {
        std::cerr << "Failed to initialize resampler" << std::endl;
        return false;
    }

    // 重采样帧通过重采样器转换得到，最终可用音频数据，信息为预期信息
    auto resampledFrame = AVCreator::createSFrame();

    std::vector<uint8_t> audioData;
    audioData.reserve(sizeInBytes());
    int totalSamples = 0;

    AVCreator::runFrameResolvingLoop(
        formatContext,
        codecContext.get(),
        audioStreamIndex,
        [this, swrCtx = swrContext.get(), resampledFrame, &out_chlayout, &audioData](AVFrame* origin_frame) {
            // 计算输出样本数
            int out_samples = av_rescale_rnd(
                swr_get_delay(swrCtx, origin_frame->sample_rate) + origin_frame->nb_samples,
                _sampleRate, origin_frame->sample_rate, AV_ROUND_UP);

            // 配置重采样输出帧
            resampledFrame->sample_rate = _sampleRate;
            resampledFrame->format = AV_SAMPLE_FMT_S16;
            av_channel_layout_copy(&resampledFrame->ch_layout, &out_chlayout);
            resampledFrame->nb_samples = out_samples;

            if (av_frame_get_buffer(resampledFrame.get(), 0) < 0) {
                std::cerr << "Failed to allocate resampled frame buffer" << std::endl;
                return 1;
            }

            int ret = swr_convert(swrCtx,
                resampledFrame->data, out_samples,
                (const uint8_t**)origin_frame->data, origin_frame->nb_samples);
            if (ret < 0) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                std::cerr << "swr_convert failed: " << errbuf << std::endl;
                return 1;
            }

            // 更新实际样本数
            resampledFrame->nb_samples = ret;

            // 处理重采样后的数据
            int data_size = ret * out_chlayout.nb_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            std::copy(
                resampledFrame->data[0],  // 源指针
                resampledFrame->data[0] + data_size,  // 源指针 + 大小
                std::back_inserter(audioData)  // 目标 vector 的插入迭代器
            );
            return 0;
        }
    );

    _pcmData = std::move(audioData);

    return true;
}