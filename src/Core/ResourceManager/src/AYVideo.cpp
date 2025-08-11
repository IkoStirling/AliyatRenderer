#include "AYVideo.h"

AYVideo::AYVideo() :
    _width(0),
    _height(0),
    _fps(0.0f),
    _duration(0.0f),
    _currentFrame(0)
{
}

AYVideo::~AYVideo()
{
    releaseData();
}

const uint8_t* AYVideo::getCurrentFramePixelData() const
{
    if (_imageData.empty()) {
        std::cerr << "[AYVideo] Current frame pixel data is empty!" << std::endl;
        return nullptr;
    }
    return _imageData.ptr();
}

bool AYVideo::updateFrame(float delta_time)
{
    if (!_loaded) return false;

    _accumulatedTime += delta_time;
    if (_accumulatedTime < _frameInterval) {
        return false;
    }
    _accumulatedTime -= _frameInterval;

    // 读取下一帧
    auto packet_deleter = [](AVPacket* p) { av_packet_free(&p); };
    auto frame_deleter = [](AVFrame* f) { av_frame_free(&f); };
    auto contex_deleter = [](SwsContext* s) { if (s) sws_freeContext(s); };

    std::unique_ptr<AVPacket, decltype(packet_deleter)>
        packet(av_packet_alloc(), packet_deleter);

    std::unique_ptr<AVFrame, decltype(frame_deleter)>
        frame(av_frame_alloc(), frame_deleter);

    auto rgbFrame = std::shared_ptr<AVFrame>(av_frame_alloc(), frame_deleter);

    //rgbFrame->format = AV_PIX_FMT_RGB24;
    rgbFrame->format = AV_PIX_FMT_RGBA; //默认转换为RGBA格式
    rgbFrame->width = _width;
    rgbFrame->height = _height;
    av_frame_get_buffer(rgbFrame.get(), 0);

    bool frameUpdated = false;

    while (av_read_frame(_formatContext, packet.get()) >= 0) {
        if (packet->stream_index == _videoStreamIndex) {
            if (avcodec_send_packet(_codecContext, packet.get()) == 0) {
                while (avcodec_receive_frame(_codecContext, frame.get()) == 0) {
                    // 转换颜色空间
                    std::unique_ptr<SwsContext, decltype(contex_deleter)> swsContext(
                        sws_getContext(
                            _width, _height, _codecContext->pix_fmt,// 输入格式（源视频的原始格式）
                            _width, _height, AV_PIX_FMT_RGBA,       // 输出格式（目标RGBA格式）
                            SWS_BILINEAR | SWS_ACCURATE_RND,        // 缩放算法和参数
                            nullptr, nullptr, nullptr),
                        contex_deleter);

                    sws_setColorspaceDetails(swsContext.get(),
                        sws_getCoefficients(SWS_CS_ITU709),  // 输入颜色空间
                        0,                                   // 输入全范围
                        sws_getCoefficients(SWS_CS_ITU709),  // 输出颜色空间
                        1,                                   // 输出全范围
                        0, 1 << 16, 1 << 16);                // 亮度/对比度参数

                    // 执行实际转换
                    int ret = sws_scale(swsContext.get(),                 // 转换上下文
                        frame->data, frame->linesize, 0, _height,   // 输入帧数据和步长
                        rgbFrame->data, rgbFrame->linesize);        // 输出帧数据和步长
                    if (ret <= 0) {
                        std::cerr << "sws_scale failed, returned: " << ret << std::endl;
                        return false;
                    }

                    // 将帧数据复制到OpenCV Mat
                    if (!rgbFrame->data[0]) {
                        std::cerr << "rgbFrame->data[0] is null!" << std::endl;
                        return false;
                    }

                    _imageData.create(_height, _width, CV_8UC4);

                    if (rgbFrame->linesize[0] != _width * 4) {
                        for (int y = 0; y < _height; y++) {
                            memcpy(_imageData.ptr(y), rgbFrame->data[0] + y * rgbFrame->linesize[0], _width * 4);
                        }
                    }
                    else {
                        memcpy(_imageData.data, rgbFrame->data[0], _width * _height * 4);
                    }

                    frameUpdated = true;
                    _currentFrame++;
                    break;
                }
            }
        }
        av_packet_unref(packet.get());
        if (frameUpdated) break;
    }

    if (frameUpdated)
    {
        // 音视频同步
        if (_audio && _audio->isStreaming()) {

        }
    }

    return frameUpdated;
}

bool AYVideo::load(const std::string& filepath)
{
    if (_loaded) return true;

    // 更新视频帧时要用，这里不进行包装
    auto format_deleter = [](AVFormatContext* ctx) { if (ctx) avformat_close_input(&ctx); };
    auto codec_deleter = [](AVCodecContext* c) { avcodec_free_context(&c); };

    // 打开视频文件
    if (avformat_open_input(&_formatContext, filepath.c_str(), nullptr, nullptr) != 0) {
        // spdlog::error("Could not open video file: {}", filepath);
        return false;
    }

    if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
        // spdlog::error("Could not find stream info: {}", filepath);
        avformat_close_input(&_formatContext);
        return false;
    }

    // 查找视频流
    _videoStreamIndex = -1;
    AVCodecParameters* codecParameters = nullptr;
    for (unsigned int i = 0; i < _formatContext->nb_streams; i++) {
        if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            _videoStreamIndex = i;
            codecParameters = _formatContext->streams[i]->codecpar;
            break;
        }
    }

    if (_videoStreamIndex == -1) {
        // spdlog::error("Could not find video stream: {}", filepath);
        avformat_close_input(&_formatContext);
        return false;
    }

    // 获取视频参数
    _width = codecParameters->width;
    _height = codecParameters->height;

    // 打开解码器
    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) {
        // spdlog::error("Unsupported codec: {}", codecParameters->codec_id);
        avformat_close_input(&_formatContext);
        return false;
    }

    _codecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(_codecContext, codecParameters) < 0) {
        // spdlog::error("Could not copy codec parameters");
        avcodec_free_context(&_codecContext);
        avformat_close_input(&_formatContext);
        return false;
    }

    if (avcodec_open2(_codecContext, codec, nullptr) < 0) {
        // spdlog::error("Could not open codec");
        avcodec_free_context(&_codecContext);
        avformat_close_input(&_formatContext);
        return false;
    }

    // 计算FPS
    AVRational timeBase = _formatContext->streams[_videoStreamIndex]->time_base;
    _fps = av_q2d(AVRational{ _codecContext->framerate.num, _codecContext->framerate.den });
    _frameInterval = 1 / _fps;
    // 计算持续时间
    _duration = static_cast<float>(_formatContext->duration) / AV_TIME_BASE;

    // 查找音频流
    _audioStreamIndex = av_find_best_stream(_formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    //if (_audioStreamIndex >= 0) {
    //    _audio = std::make_unique<AYAudioStream>();
    //    _audio->load(filepath);
    //}

    IAYResource::load(filepath);
    _loaded = true;
    return true;
}

bool AYVideo::unload()
{
    if (!_loaded) return true;
    std::lock_guard<std::mutex> lock(_dataMutex);
    releaseData();
    _loaded = false;
    return true;
}

bool AYVideo::reload(const std::string& filepath)
{
    unload();
    return load(filepath);
}

size_t AYVideo::sizeInBytes()
{
    // 估算视频数据大小 (宽度 * 高度 * 24bit/像素 * 帧数)
    // 注意: 这是一个粗略估计，实际视频文件大小可能不同
    size_t size = 0;
    size += static_cast<size_t>(_width * _height * 3 * _fps * _duration);
    if (_audio) size += _audio->sizeInBytes();
    return size;
}

void AYVideo::releaseData()
{
    //if (_audio && _audio->getPCMData()!=std::vector<uint8_t>())
    //    _audio->unload();
    if (_codecContext) {
        avcodec_free_context(&_codecContext);
        _codecContext = nullptr;
    }
    if (_formatContext) {
        avformat_close_input(&_formatContext);
        _formatContext = nullptr;
    }
    _imageData.release();
}

