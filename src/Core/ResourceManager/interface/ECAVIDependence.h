#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <iostream>
#include <atomic>
#include <queue>
#include <memory>
#include <functional>
extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/opt.h> 
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#include <opencv2/opencv.hpp>

struct AudioFrame;
struct AVPacketDeleter;
struct AVFrameDeleter;
struct AVCodecContextDeleter;
struct SwrContextDeleter;
struct SwsContextDeleter;
class AVFormatContextHolder;

struct AVPacketDeleter {
    void operator()(AVPacket* p) const {
        av_packet_free(&p);
    }
};

struct AVFrameDeleter {
    void operator()(AVFrame* f) const {
        av_frame_free(&f);
    }
};
static AVFrameDeleter globalFrameDelter;

struct AVCodecContextDeleter {
    void operator()(AVCodecContext* ctx) const {
        if (ctx) avcodec_free_context(&ctx);
    }
};

struct SwrContextDeleter {
    void operator()(SwrContext* swr) const {
        if (swr) swr_free(&swr);
    }
};
struct SwsContextDeleter {
    void operator()(SwsContext* s) const {
        if (s) sws_freeContext(s);
    }
};


using AVFramePtr = std::unique_ptr<AVFrame, AVFrameDeleter>;
using AVFrameSPtr = std::shared_ptr<AVFrame>;
using AVPacketPtr = std::unique_ptr<AVPacket, AVPacketDeleter>;
using AVCodecCtxPtr = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;
using AVSwrCtxPtr = std::unique_ptr<SwrContext, SwrContextDeleter>;
using AVSwsCtxPtr = std::unique_ptr<SwsContext, SwsContextDeleter>;
using AVFormatCtxPtr = std::shared_ptr<AVFormatContextHolder>;
using AudioFramePtr = std::shared_ptr<AudioFrame>;

struct AudioFrame {
    std::vector<uint8_t> data;
    double pts;
    ALuint alBuffer = 0;
};



class AVFormatContextHolder 
{
public:
    AVFormatContextHolder(const std::string& path) {
        if (avformat_open_input(&ctx, path.c_str(), nullptr, nullptr) != 0) {
            std::cerr << "Failed to open input file: " << path << std::endl;
        }
    }
    ~AVFormatContextHolder() {
        if (ctx) avformat_close_input(&ctx);
    }
    AVFormatContext* get() { return ctx; }

    int findStream(AVCodecParameters*& codecParameters, AVMediaType type)
    {
        if (!ctx) {
            std::cerr << "Error: AVFormatContext is null. Did you open the file successfully?" << std::endl;
            codecParameters = nullptr;
            return -1;
        }

        codecParameters = nullptr;

        if (avformat_find_stream_info(ctx, nullptr) < 0) {
            std::cerr << "Failed to find stream info" << std::endl;
            return -1;
        }

        for (unsigned int i = 0; i < ctx->nb_streams; i++) {
            if (ctx->streams[i]->codecpar->codec_type == type) {
                codecParameters = ctx->streams[i]->codecpar;
                return i;
            }
        }

        return -1;
    }
private:
    AVFormatContext* ctx = nullptr;
};

class AVCreator
{
public:
    static AVFormatCtxPtr createFormatContext(const std::string& path) {
        return std::make_shared<AVFormatContextHolder>(path);
    }
    static AVFramePtr createFrame() {
        return AVFramePtr(av_frame_alloc());
    }
    static AVFrameSPtr createSFrame() {
        return AVFrameSPtr(av_frame_alloc(), globalFrameDelter);
    }
    static AVPacketPtr createPacket() {
        return AVPacketPtr(av_packet_alloc());
    }
    static AVCodecCtxPtr createCodecContext(const AVCodec* codec) {
        return AVCodecCtxPtr(avcodec_alloc_context3(codec));
    }
    static AVSwrCtxPtr createSwrContext() {
        return AVSwrCtxPtr(swr_alloc());
    }
    static AVSwsCtxPtr createSwsContext(SwsContext* originPtr) {
        return AVSwsCtxPtr(originPtr);
    }

    static const AVCodec* findDecoder(AVCodecParameters* codecParameters) {
        const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            std::cerr << "Unsupported codec: " << avcodec_get_name(codecParameters->codec_id) << std::endl;
            return nullptr;
        }
        return codec;
    }

    // 返回0为成功，1为跳过当前循环，-1为失败
    static void runFrameResolvingLoop(
        AVFormatCtxPtr formatCtx,
        AVCodecContext* codecCtx,
        int stream_index,
        std::function<int(AVFrame* origin_frame)> operation,
        bool updateOnce = false)
    {
        auto packet = AVCreator::createPacket();
        auto origin_frame = AVCreator::createFrame();
        int flag = -1; 
        while (av_read_frame(formatCtx->get(), packet.get()) >= 0) {
            if (packet->stream_index == stream_index) {
                if (avcodec_send_packet(codecCtx, packet.get()) == 0) {
                    while (avcodec_receive_frame(codecCtx, origin_frame.get()) == 0) {
                        flag = operation(origin_frame.get());
                        if (flag == -1)
                        {
                            std::cerr << "[AVCreator] runFrameResolvingLoop failed \n";
                            return;
                        }
                        else if (flag == 1)
                        {
                            continue;
                        }
                    }
                }
            }
            av_packet_unref(packet.get());
            if (flag==0 && updateOnce)
                return;
        }
    }

};




