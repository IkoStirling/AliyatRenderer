#include "AYAudioStream.h"
#include "AYPath.h"

AYAudioStream::AYAudioStream()
{

}

AYAudioStream::~AYAudioStream() {
    unload();
}

bool AYAudioStream::open(const std::string& path) {
    return load(path);
}

bool AYAudioStream::load(const std::string& filepath) {
    if (_fmtCtx) unload();

    std::string rpath = AYPath::resolve(filepath);

    _fmtCtx = AVCreator::createFormatContext(rpath);
    if (!_fmtCtx || !_fmtCtx->get()) return false;

    // 查找音频流
    AVCodecParameters* codecPar = nullptr;
    _streamIndex = _fmtCtx->findStream(codecPar, AVMEDIA_TYPE_AUDIO);
    if (_streamIndex < 0 || !codecPar) return false;

    // 初始化解码器
    if (!initDecoder()) return false;

    // 获取音频参数, 流式音频不考虑3D音效，通道不做处理
    _sampleRate = codecPar->sample_rate;
    _channels = codecPar->ch_layout.nb_channels;
    _format = (_channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    // 计算持续时间
    if (_fmtCtx->get()->duration != AV_NOPTS_VALUE) {
        _duration = static_cast<float>(_fmtCtx->get()->duration) / AV_TIME_BASE;
    }

    // 初始化重采样器
    if (!initResampler()) return false;

    return IAYResource::load(rpath);
}

bool AYAudioStream::initDecoder() {
    auto codecPar = _fmtCtx->get()->streams[_streamIndex]->codecpar;
    const AVCodec* codec = AVCreator::findDecoder(codecPar);
    if (!codec) return false;

    _codecCtx = AVCreator::createCodecContext(codec);
    if (avcodec_parameters_to_context(_codecCtx.get(), codecPar) < 0) {
        return false;
    }

    return avcodec_open2(_codecCtx.get(), codec, nullptr) == 0;
}

bool AYAudioStream::initResampler() {
    _swrCtx = AVCreator::createSwrContext();

    // 输入参数（源音频格式）
    av_opt_set_chlayout(_swrCtx.get(), "in_chlayout", &_codecCtx->ch_layout, 0);
    av_opt_set_int(_swrCtx.get(), "in_sample_rate", _codecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(_swrCtx.get(), "in_sample_fmt", _codecCtx->sample_fmt, 0);

    // 输出参数（OpenAL兼容格式）
    AVChannelLayout outLayout;
    if (_channels == 1)
        outLayout = AV_CHANNEL_LAYOUT_MONO;
    else
        outLayout = AV_CHANNEL_LAYOUT_STEREO;
    av_opt_set_chlayout(_swrCtx.get(), "out_chlayout", &outLayout, 0);
    av_opt_set_int(_swrCtx.get(), "out_sample_rate", _sampleRate, 0);
    av_opt_set_sample_fmt(_swrCtx.get(), "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    return swr_init(_swrCtx.get()) >= 0;
}

AudioFramePtr AYAudioStream::decodeNextFrame() {
    std::lock_guard<std::mutex> lock(_decodeMutex);

    if (!_fmtCtx || !_codecCtx) return nullptr;

    auto frame = AVCreator::createFrame();
    auto packet = AVCreator::createPacket();
    AudioFramePtr audioFrame;

    AVCreator::runFrameResolvingLoop(_fmtCtx, _codecCtx.get(), _streamIndex,
        [this, &audioFrame](AVFrame* origin_frame) {
            int outSamples = swr_get_out_samples(_swrCtx.get(), origin_frame->nb_samples);
            auto resampled = AVCreator::createFrame();
            resampled->sample_rate = _sampleRate;
            resampled->format = AV_SAMPLE_FMT_S16;
            av_channel_layout_copy(&resampled->ch_layout, &_codecCtx->ch_layout);
            resampled->nb_samples = outSamples;
            av_frame_get_buffer(resampled.get(), 0);

            swr_convert(_swrCtx.get(),
                resampled->data, outSamples,
                (const uint8_t**)origin_frame->data, origin_frame->nb_samples);

            // 封装音频帧
            audioFrame = std::make_shared<AudioFrame>();
            audioFrame->data.assign(
                resampled->data[0],
                resampled->data[0] + outSamples * _channels * 2);
            audioFrame->pts = origin_frame->pts * av_q2d(_fmtCtx->get()->streams[_streamIndex]->time_base);
            return 0;
        },
        true
    );

    return audioFrame;
}

bool AYAudioStream::seek(double seconds) {
    std::lock_guard<std::mutex> lock(_decodeMutex);

    if (!_fmtCtx || !_codecCtx) return false;

    // 清空解码器缓冲区
    avcodec_flush_buffers(_codecCtx.get());

    // 计算时间戳
    int64_t ts = seconds / av_q2d(_fmtCtx->get()->streams[_streamIndex]->time_base);

    return av_seek_frame(_fmtCtx->get(), _streamIndex, ts, AVSEEK_FLAG_BACKWARD) >= 0;
}


bool AYAudioStream::unload() {
    std::lock_guard<std::mutex> lock(_decodeMutex);
    releaseData();
    return true;
}

bool AYAudioStream::reload(const std::string& filepath)
{
    std::string rpath = AYPath::resolve(filepath);
    unload();
    return load(rpath);
}

void AYAudioStream::releaseData() {
    _swrCtx.reset();
    _codecCtx.reset();
    _fmtCtx.reset();

    // 清空帧队列
    while (!_frameQueue.empty()) {
        _frameQueue.pop();
    }
}

size_t AYAudioStream::sizeInBytes() {
    // 流式音频无法准确计算大小，返回预估内存占用
    return sizeof(*this) + (_frameQueue.size() * 1024); // 假设每帧约1KB
}