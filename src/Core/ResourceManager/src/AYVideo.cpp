#include "AYVideo.h"
#include "AYResourceManager.h"
#include <libavutil/time.h>

AYVideo::AYVideo() :
	_width(0),
	_height(0),
	_fps(0.0f),
	_duration(0.0f)
{
}

AYVideo::~AYVideo()
{
	releaseData();
}

const cv::Mat& AYVideo::getCurrentFrameData() const
{
	static cv::Mat placeholder;
	static std::once_flag flag;
	std::call_once(flag, [this]() {
		// 使用实际视频尺寸创建占位符
		int width = _width > 0 ? _width : 640;
		int height = _height > 0 ? _height : 480;
		placeholder.create(height, width, CV_8UC4);
		placeholder.setTo(cv::Scalar(0, 0, 0, 255));
		});

	std::lock_guard<std::recursive_mutex> lock(_dataMutex);

	if (_currentFrame.empty())
		return placeholder;
	else
		return _currentFrame;
}

const uint8_t* AYVideo::getCurrentFramePixelData() const
{
	return getCurrentFrameData().ptr();
}

void AYVideo::stop()
{
	std::lock_guard<std::recursive_mutex> lock(_dataMutex);
	_isPlaying = false;
	_isEndOfVideo = false;
	_currentFrameIndex = 0;

	if (_formatContext && _videoStreamIndex >= 0) {
		av_seek_frame(_formatContext, _videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
		avcodec_flush_buffers(_codecContext);
		decodeNextFrame();
	}
}


void AYVideo::updateFrame(float delta_time)
{
	std::lock_guard<std::recursive_mutex> lock(_dataMutex);

	if (!_isPlaying || _isEndOfVideo) {
		return;
	}

	// 更新累计时间
	static float accumulated_time = 0.0f;
	accumulated_time += delta_time;

	// 计算当前应该显示的帧索引
	float frame_interval = 1.0f / _fps;
	int frames_to_advance = static_cast<int>(accumulated_time / frame_interval);

	if (frames_to_advance > 0) {
		accumulated_time -= frames_to_advance * frame_interval;

		// 处理音频同步
		//if (_audio && _audio->isPlaying()) {
		//    double audio_time = _audio->getCurrentTime();
		//    double video_time = _currentFrameIndex / _fps;

		//    // 简单的同步策略：如果视频落后音频超过1帧，跳帧
		//    if (video_time < audio_time - frame_interval) {
		//        int target_frame = static_cast<int>(audio_time * _fps);
		//        seekToFrame(target_frame);
		//        return;
		//    }
		//    // 如果视频超前音频超过1帧，等待
		//    else if (video_time > audio_time + frame_interval) {
		//        return;
		//    }
		//}

		// 前进帧
		for (int i = 0; i < frames_to_advance; ++i) {
			if (!advanceFrame()) {
				break;
			}
		}
	}
}
bool AYVideo::advanceFrame()
{
	std::lock_guard<std::recursive_mutex> lock(_dataMutex);

	if (_isEndOfVideo) {
		return false;
	}

	if (!decodeNextFrame()) {
		spdlog::info("[AYVideo] decode failed or end");
		return false;
	}

	_currentFrameIndex++;

	if (_currentFrameIndex >= _totalFrames) {
		_isEndOfVideo = true;
	}

	return true;
}

bool AYVideo::seekToTime(float seconds) {
	std::lock_guard<std::recursive_mutex> lock(_dataMutex);

	if (!_formatContext || seconds < 0 || seconds > _duration) {
		return false;
	}

	// 计算时间戳
	AVRational timeBase = _formatContext->streams[_videoStreamIndex]->time_base;
	int64_t targetTimestamp = av_rescale_q(
		static_cast<int64_t>(seconds * AV_TIME_BASE),
		AV_TIME_BASE_Q,
		timeBase);

	return seekToTimestamp(targetTimestamp);
}

bool AYVideo::seekToFrame(int frameIndex) {
	std::lock_guard<std::recursive_mutex> lock(_dataMutex);

	if (frameIndex < 0 || frameIndex >= _totalFrames) {
		return false;
	}

	// 计算时间戳
	AVRational timeBase = _formatContext->streams[_videoStreamIndex]->time_base;
	int64_t targetTimestamp = av_rescale_q(frameIndex,
		av_inv_q(_formatContext->streams[_videoStreamIndex]->avg_frame_rate),
		timeBase);

	return seekToTimestamp(targetTimestamp);
}

bool AYVideo::seekToTimestamp(int64_t timestamp) {
	if (av_seek_frame(_formatContext, _videoStreamIndex,
		timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
		return false;
	}

	avcodec_flush_buffers(_codecContext);
	_isEndOfVideo = false;

	// 解码下一帧
	if (!decodeNextFrame()) {
		return false;
	}

	// 更新当前帧索引 (近似计算)
	_currentFrameIndex = static_cast<int>((timestamp * _fps *
		_formatContext->streams[_videoStreamIndex]->time_base.num) /
		_formatContext->streams[_videoStreamIndex]->time_base.den);

	return true;
}

bool AYVideo::decodeNextFrame()
{
	AVPacketPtr packet = AVCreator::createPacket();
	AVFramePtr decodedFrame = AVCreator::createFrame();
	bool frameDecoded = false;
	int ret = 0;

	while (!frameDecoded) {
		// 1. 尝试从解码器获取已解码的帧
		ret = avcodec_receive_frame(_codecContext, decodedFrame.get());
		if (ret == 0) {
			frameDecoded = true;
			break;
		}
		else if (ret == AVERROR(EAGAIN)) {
			// 需要更多数据，继续读取
		}
		else if (ret == AVERROR_EOF) {
			// 解码器已刷新，需要seek回到开始位置
			av_seek_frame(_formatContext, _videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(_codecContext);
			continue;
		}
		else {
			// 其他错误，重置解码器
			avcodec_flush_buffers(_codecContext);
			continue;
		}

		// 2. 读取新的数据包
		ret = av_read_frame(_formatContext, packet.get());
		if (ret < 0) {
			if (ret == AVERROR_EOF) {
				// 文件结束，发送flush包
				avcodec_send_packet(_codecContext, nullptr);
				continue;
			}
			// 读取错误，重置解码器
			avcodec_flush_buffers(_codecContext);
			continue;
		}

		// 3. 发送数据包到解码器
		if (packet->stream_index == _videoStreamIndex) {
			ret = avcodec_send_packet(_codecContext, packet.get());
			if (ret < 0 && ret != AVERROR(EAGAIN)) {
				// 发送失败，重置解码器
				avcodec_flush_buffers(_codecContext);
			}
		}
		av_packet_unref(packet.get());
	}

	if (!frameDecoded) {
		return false;
	}

	// 4. 转换帧格式
	AVFrameSPtr rgbFrame = AVCreator::createSFrame();
	rgbFrame->format = AV_PIX_FMT_RGBA;
	rgbFrame->width = _width;
	rgbFrame->height = _height;

	if (av_frame_get_buffer(rgbFrame.get(), 0) < 0) {
		return false;
	}

	// 创建SWS上下文
	auto swsContext = AVCreator::createSwsContext(
		sws_getContext(
			_width, _height, _codecContext->pix_fmt,
			_width, _height, AV_PIX_FMT_RGBA,
			SWS_BILINEAR | SWS_ACCURATE_RND,
			nullptr, nullptr, nullptr)
	);

	if (!swsContext) {
		return false;
	}

	// 设置颜色空间参数
	const int* coeffs = sws_getCoefficients(SWS_CS_ITU709);
	sws_setColorspaceDetails(swsContext.get(),
		coeffs, _codecContext->color_range == AVCOL_RANGE_JPEG,
		coeffs, 1, 0, 1 << 16, 1 << 16);

	// 执行转换
	if (sws_scale(swsContext.get(),
		decodedFrame->data, decodedFrame->linesize, 0, _height,
		rgbFrame->data, rgbFrame->linesize) <= 0) {
		return false;
	}

	// 5. 转换为cv::Mat
	return convertFrameToMat(rgbFrame.get());
}

bool AYVideo::convertFrameToMat(AVFrame* frame)
{
	if (!frame || !frame->data[0]) {
		spdlog::error("[AYVideo] Invalid frame data");
		return false;
	}

	try {
		_currentFrame.create(_height, _width, CV_8UC4);

		if (frame->linesize[0] != _width * 4) {
			for (int y = 0; y < _height; y++) {
				memcpy(_currentFrame.ptr(y), frame->data[0] + y * frame->linesize[0], _width * 4);
			}
		}
		else {
			memcpy(_currentFrame.data, frame->data[0], _width * _height * 4);
		}
		return true;
	}
	catch (const cv::Exception& e) {
		spdlog::error("[AYVideo] OpenCV exception: {}", e.what());
		return false;
	}
}

void AYVideo::setSyncCallback(SyncCallback callback)
{
	_syncCallback = callback;
}

void AYVideo::syncToAudio(double sync_time)
{
	if (!_syncCallback) return;

	double videoTime = _currentFrameIndex / _fps;
	double diff = videoTime - sync_time;

	// 如果差异超过阈值（如40ms），调整视频
	if (std::abs(diff) > 0.04) {
		int targetFrame = static_cast<int>(sync_time * _fps);
		seekToFrame(targetFrame);
	}
}

bool AYVideo::load(const std::string& filepath)
{
	if (_loaded) return true;

	// 更新视频帧时要用，这里不进行包装
	auto format_deleter = [](AVFormatContext* ctx) { if (ctx) avformat_close_input(&ctx); };
	auto codec_deleter = [](AVCodecContext* c) { avcodec_free_context(&c); };

	// 打开视频文件
	if (avformat_open_input(&_formatContext, filepath.c_str(), nullptr, nullptr) != 0) {
		spdlog::error("[AYVideo] Could not open video file: {}", filepath);
		return false;
	}

	if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
		spdlog::error("[AYVideo] Could not find stream info: {}", filepath);
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
		spdlog::error("[AYVideo] Could not find video stream: {}", filepath);
		avformat_close_input(&_formatContext);
		return false;
	}

	// 打开解码器
	const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
	if (!codec) {
		spdlog::error("[AYVideo] Unsupported codec: {}", static_cast<int>(codecParameters->codec_id));
		avformat_close_input(&_formatContext);
		return false;
	}

	_codecContext = avcodec_alloc_context3(codec);
	if (avcodec_parameters_to_context(_codecContext, codecParameters) < 0 ||
		avcodec_open2(_codecContext, codec, nullptr) < 0)
	{
		avcodec_free_context(&_codecContext);
		avformat_close_input(&_formatContext);
		return false;
	}

	_width = codecParameters->width;
	_height = codecParameters->height;
	_fps = av_q2d(_formatContext->streams[_videoStreamIndex]->avg_frame_rate);
	_duration = static_cast<float>(_formatContext->duration) / AV_TIME_BASE;

	// 查找音频流
	_audioStreamIndex = av_find_best_stream(_formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (_audioStreamIndex >= 0) {
		_audio = std::make_shared<AYAudioStream>();
		_audio->load(filepath);
	}

	if (_formatContext->streams[_videoStreamIndex]->nb_frames > 0) {
		_totalFrames = static_cast<int>(_formatContext->streams[_videoStreamIndex]->nb_frames);
	}
	else {
		// 估算帧数
		_totalFrames = static_cast<int>(_duration * _fps);
	}
	_isEndOfVideo = false;
	_currentFrameIndex = 0;

	// 初始化解码器并解码第一帧
	av_seek_frame(_formatContext, _videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(_codecContext);
	if (!decodeNextFrame()) {
		releaseData();
		return false;
	}

	IAYResource::load(filepath);
	_loaded = true;
	return true;
}

bool AYVideo::unload()
{
	if (!_loaded) return true;

	std::lock_guard<std::recursive_mutex> lock(_dataMutex);
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
	_currentFrame.release();
}




