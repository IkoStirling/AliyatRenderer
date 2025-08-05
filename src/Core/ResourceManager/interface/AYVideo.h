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
    #include <libswscale/swscale.h>
}
#include <opencv2/opencv.hpp>

class AYVideo : public IAYResource
{
    SUPPORT_MEMORY_POOL(AYVideo)
    DECLARE_RESOURCE_CLASS(AYVideo, "videos")
public:
    AYVideo() :
        _textureID(0),
        _width(0),
        _height(0),
        _fps(0.0f),
        _duration(0.0f),
        _currentFrame(0)
    {
    }

    virtual ~AYVideo() {
        releaseData();
    }

    // 获取纹理ID
    GLuint getTextureID() const { return _textureID; }

    // 获取视频信息
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    float getFPS() const { return _fps; }
    float getDuration() const { return _duration; }

    // 更新当前帧到纹理
    bool updateFrame() {
        if (!_loaded) return false;

        // 读取下一帧
        AVPacket* packet = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();
        AVFrame* rgbFrame = av_frame_alloc();

        rgbFrame->format = AV_PIX_FMT_RGB24;
        rgbFrame->width = _width;
        rgbFrame->height = _height;
        av_frame_get_buffer(rgbFrame, 0);

        bool frameUpdated = false;

        while (av_read_frame(_formatContext, packet) >= 0) {
            if (packet->stream_index == _videoStreamIndex) {
                if (avcodec_send_packet(_codecContext, packet) == 0) {
                    while (avcodec_receive_frame(_codecContext, frame) == 0) {
                        // 转换颜色空间
                        SwsContext* swsContext = sws_getContext(
                            _width, _height, _codecContext->pix_fmt,
                            _width, _height, AV_PIX_FMT_RGB24,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);

                        if (swsContext) {
                            sws_scale(swsContext,
                                frame->data, frame->linesize, 0, _height,
                                rgbFrame->data, rgbFrame->linesize);
                            sws_freeContext(swsContext);

                            // 将帧数据复制到OpenCV Mat
                            _imageData = cv::Mat(_height, _width, CV_8UC3, rgbFrame->data[0]);

                            // 转换为RGBA格式
                            cv::cvtColor(_imageData, _imageData, cv::COLOR_RGB2RGBA);

                            // 上传到GPU
                            if (_textureID != 0) {
                                glBindTexture(GL_TEXTURE_2D, _textureID);
                                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height,
                                    GL_RGBA, GL_UNSIGNED_BYTE, _imageData.data);
                            }
                            else {
                                // 首次加载，创建纹理
                                _textureID = 0;
                                glGenTextures(1, &_textureID);
                                glBindTexture(GL_TEXTURE_2D, _textureID);
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                                    _width, _height, 0,
                                    GL_RGBA, GL_UNSIGNED_BYTE, _imageData.data);

                                // 设置纹理参数
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                            }

                            frameUpdated = true;
                            _currentFrame++;
                            break;
                        }
                    }
                }
            }
            av_packet_unref(packet);
            if (frameUpdated) break;
        }

        av_frame_free(&rgbFrame);
        av_frame_free(&frame);
        av_packet_free(&packet);

        return frameUpdated;
    }

    virtual bool load(const std::string& filepath) override
    {
        if (_loaded) return true;

        // 初始化FFmpeg
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

        // 计算持续时间
        _duration = static_cast<float>(_formatContext->duration) / AV_TIME_BASE;

        // 初始化纹理
        _textureID = 0;

        IAYResource::load(filepath);
        _loaded = true;
        return true;
    }

    virtual bool unload() override
    {
        if (!_loaded) return true;
        std::lock_guard<std::mutex> lock(_dataMutex);
        if (_textureID != 0) {
            glDeleteTextures(1, &_textureID);
            _textureID = 0;
        }
        if (_codecContext) {
            avcodec_free_context(&_codecContext);
            _codecContext = nullptr;
        }
        if (_formatContext) {
            avformat_close_input(&_formatContext);
            _formatContext = nullptr;
        }
        _imageData.release();
        _loaded = false;
        return true;
    }

    virtual bool reload(const std::string& filepath) override
    {
        unload();
        return load(filepath);
    }

    virtual size_t sizeInBytes() override
    {
        // 估算视频数据大小 (宽度 * 高度 * 3字节/像素 * 帧数)
        // 注意: 这是一个粗略估计，实际视频文件大小可能不同
        return static_cast<size_t>(_width * _height * 3 * _fps * _duration);
    }

private:
    void releaseData() {
        GLuint id = _textureID;
        _textureID = 0;
        if (id != 0) {
            glDeleteTextures(1, &id);
        }
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

    GLuint _textureID;
    AVFormatContext* _formatContext = nullptr;
    AVCodecContext* _codecContext = nullptr;
    int _videoStreamIndex = -1;
    int _width;
    int _height;
    float _fps;
    float _duration;
    int _currentFrame;
    cv::Mat _imageData;
    mutable std::mutex _dataMutex;
};

REGISTER_RESOURCE_CLASS(AYVideo, 0)