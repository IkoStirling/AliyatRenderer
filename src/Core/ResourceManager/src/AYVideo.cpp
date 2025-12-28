#include "AYVideo.h"
#include "AYResourceManager.h"
#include <libavutil/time.h>
#include "AYLogger.h"
namespace ayt::engine::resource
{
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

		if (_displayQueue.empty()) {
			return placeholder; // 返回空白帧
		}

		AVFrameSPtr frame = _displayQueue.back();

		if (convertFrameToMat(frame.get()))
			return _currentFrame;
		else
			return placeholder;
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

		if (_formatContext->get() && _videoStreamIndex >= 0) {
			av_seek_frame(_formatContext->get(), _videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(_codecContext.get());
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
			AYLOG_INFO("[AYVideo] decode failed or end");
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

		if (!_formatContext->get() || seconds < 0 || seconds > _duration) {
			return false;
		}

		// 计算时间戳
		AVRational timeBase = _formatContext->get()->streams[_videoStreamIndex]->time_base;
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
		AVRational timeBase = _formatContext->get()->streams[_videoStreamIndex]->time_base;
		int64_t targetTimestamp = av_rescale_q(frameIndex,
			av_inv_q(_formatContext->get()->streams[_videoStreamIndex]->avg_frame_rate),
			timeBase);

		return seekToTimestamp(targetTimestamp);
	}

	bool AYVideo::seekToTimestamp(int64_t timestamp) {
		avcodec_flush_buffers(_codecContext.get());
		_displayQueue.clear();

		if (av_seek_frame(_formatContext->get(), _videoStreamIndex,
			timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
			return false;
		}


		_isEndOfVideo = false;

		// 解码下一帧
		AVFrameSPtr frame = AVCreator::createFrame();
		while (decodeNextFrame()) {
			frame = _displayQueue.back();
			if (frame->pts >= timestamp) { // 找到目标帧或其后第一帧
				_currentFrameIndex = static_cast<int>((timestamp * _fps *
					_formatContext->get()->streams[_videoStreamIndex]->time_base.num) /
					_formatContext->get()->streams[_videoStreamIndex]->time_base.den);
				break;
			}
		}
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
			ret = avcodec_receive_frame(_codecContext.get(), decodedFrame.get());
			if (ret == 0) {
				frameDecoded = true;
				//if (decodedFrame.get()->pict_type == AV_PICTURE_TYPE_I) {
				//	printf("解码出的帧是I帧。\n");
				//}
				//else if (decodedFrame.get()->pict_type == AV_PICTURE_TYPE_P) {
				//	printf("解码出的帧是P帧。\n");
				//}
				//else if (decodedFrame.get()->pict_type == AV_PICTURE_TYPE_B) {
				//	printf("解码出的帧是B帧。\n");
				//}
				break;
			}
			else if (ret == AVERROR(EAGAIN)) {
				// 需要更多数据，继续读取
			}
			else if (ret == AVERROR_EOF) {
				// 解码器已刷新，需要seek回到开始位置
				av_seek_frame(_formatContext->get(), _videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
				avcodec_flush_buffers(_codecContext.get());
				continue;
			}
			else {
				// 其他错误，重置解码器
				avcodec_flush_buffers(_codecContext.get());
				continue;
			}

			// 2. 读取新的数据包
			ret = av_read_frame(_formatContext->get(), packet.get());
			//if (packet.get()->flags & AV_PKT_FLAG_KEY) {
			//	printf("这是一个关键帧！\n");
			//}
			if (ret < 0) {
				if (ret == AVERROR_EOF) {
					// 文件结束，发送flush包
					avcodec_send_packet(_codecContext.get(), nullptr);
					continue;
				}
				// 读取错误，重置解码器
				avcodec_flush_buffers(_codecContext.get());
				continue;
			}

			// 3. 发送数据包到解码器
			if (packet->stream_index == _videoStreamIndex) {
				ret = avcodec_send_packet(_codecContext.get(), packet.get());
				if (ret < 0 && ret != AVERROR(EAGAIN)) {
					// 发送失败，重置解码器
					avcodec_flush_buffers(_codecContext.get());
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

		if (sws_scale(_swsContext.get(),
			decodedFrame->data, decodedFrame->linesize, 0, _height,
			rgbFrame->data, rgbFrame->linesize) <= 0) {
			return false;
		}

		if (_displayQueue.size() >= MAX_DISPLAY_QUEUE_SIZE) {
			_displayQueue.pop_front(); // 丢弃最旧的帧
		}
		_displayQueue.push_back(rgbFrame); // 存入显示队列
		return true;
	}

	bool AYVideo::convertFrameToMat(AVFrame* frame) const
	{
		if (!frame || !frame->data[0]) {
			AYLOG_ERR("[AYVideo] Invalid frame data");
			return false;
		}

		try {
			if (_currentFrame.empty() || _currentPts != frame->pts)
			{
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
		}
		catch (const cv::Exception& e) {
			AYLOG_ERR("[AYVideo] OpenCV exception: {}", e.what());
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
		if (std::abs(diff) > 0.1) { // 超过 100ms 不同步
			int targetFrame = static_cast<int>(sync_time * _fps);
			seekToFrame(targetFrame);
		}
		else if (diff > 0) {
			//_playbackSpeed = 0.95f; // 视频比音频快，轻微降速
		}
		else {
			//_playbackSpeed = 1.05f; // 视频比音频慢，轻微加速
		}
	}

	bool AYVideo::load(const std::string& filepath)
	{
		if (_loaded) return true;

		// 更新视频帧时要用，这里不进行包装
		auto format_deleter = [](AVFormatContext* ctx) { if (ctx) avformat_close_input(&ctx); };
		auto codec_deleter = [](AVCodecContext* c) { avcodec_free_context(&c); };

		// 打开视频文件
		_formatContext = std::make_shared<AVFormatContextHolder>();

		if (!_formatContext->open(filepath)) {
			AYLOG_ERR("[AYVideo] Could not open video file: {}", filepath);
			return false;
		}

		// 查找视频流
		_videoStreamIndex = -1;
		AVCodecParameters* codecParameters = nullptr;
		_videoStreamIndex = _formatContext->findStream(codecParameters, AVMEDIA_TYPE_VIDEO);
		if (_videoStreamIndex == -1) {
			AYLOG_ERR("[AYVideo] Could not find video stream: {}", filepath);
			_formatContext->reset();
			return false;
		}

		// 打开解码器
		const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
		if (!codec) {
			AYLOG_ERR("[AYVideo] Unsupported codec: {}", static_cast<int>(codecParameters->codec_id));
			_formatContext->reset();
			return false;
		}

		_codecContext = AVCreator::createCodecContext(codec);
		if (avcodec_parameters_to_context(_codecContext.get(), codecParameters) < 0 ||
			avcodec_open2(_codecContext.get(), codec, nullptr) < 0)
		{
			_codecContext.reset();
			_formatContext->reset();
			return false;
		}

		_width = codecParameters->width;
		_height = codecParameters->height;
		_fps = av_q2d(_formatContext->get()->streams[_videoStreamIndex]->avg_frame_rate);
		_duration = static_cast<float>(_formatContext->get()->duration) / AV_TIME_BASE;

		_swsContext = AVCreator::createSwsContext(
			sws_getContext(
				_width, _height, _codecContext.get()->pix_fmt,
				_width, _height, AV_PIX_FMT_RGBA,
				SWS_BILINEAR | SWS_ACCURATE_RND,
				nullptr, nullptr, nullptr)
		);

		// 设置颜色空间参数
		const int* coeffs = sws_getCoefficients(SWS_CS_ITU709);
		sws_setColorspaceDetails(_swsContext.get(),
			coeffs, _codecContext.get()->color_range == AVCOL_RANGE_JPEG,
			coeffs, 1, 0, 1 << 16, 1 << 16);

		// 查找音频流
		_audioStreamIndex = _formatContext->findBsetStream(codecParameters, AVMEDIA_TYPE_AUDIO);
		if (_audioStreamIndex >= 0) {
			_audio = std::make_shared<AYAudioStream>();
			_audio->load(filepath);
		}

		if (_formatContext->get()->streams[_videoStreamIndex]->nb_frames > 0) {
			_totalFrames = static_cast<int>(_formatContext->get()->streams[_videoStreamIndex]->nb_frames);
		}
		else {
			// 估算帧数
			_totalFrames = static_cast<int>(_duration * _fps);
		}
		_isEndOfVideo = false;
		_currentFrameIndex = 0;

		// 初始化解码器并解码第一帧
		av_seek_frame(_formatContext->get(), _videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
		avcodec_flush_buffers(_codecContext.get());
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
		size += static_cast<size_t>(_width * _height * 3 * MAX_DISPLAY_QUEUE_SIZE);
		if (_audio) size += _audio->sizeInBytes();
		return size;
	}

	void AYVideo::releaseData()
	{
		_currentFrame.release();
	}




}