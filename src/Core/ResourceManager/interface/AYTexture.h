#pragma once
#include "IAYResource.h"
#include "AYResourceRegistry.h"
#include "AYMemoryPool.h"
#include <iostream>
#include <atomic>
#include "opencv2/opencv.hpp"


/*
	移除所有非OpenGL操作，仅提供原始数据
*/
class AYTexture : public IAYResource
{
	SUPPORT_MEMORY_POOL(AYTexture)
	DECLARE_RESOURCE_CLASS(AYTexture, "textures")
public:
	enum class TextureFormat {
		RGBA8,
		RGB8,
		R8,
		RGBA16F,
		Depth32F
	};

	AYTexture() :
		_format(TextureFormat::RGBA8),
		_width(0),
		_height(0),
		_channels(0)
	{
	}
	AYTexture(unsigned int format) :
		_format((TextureFormat)(format)),
		_width(0),
		_height(0),
		_channels(0) 
	{}

	virtual ~AYTexture() {
		releaseData();
	}


	// 获取纹理尺寸
	int getWidth() const { return _width; }
	int getHeight() const { return _height; }
	int getChannels() const { return _channels; }

	//获取OpenCV格式的图像数据
	const cv::Mat& getImageData() const { return _imageData; }

	const uint8_t* getPixelData() const {
		return _imageData.ptr();
	}

	virtual bool load(const std::string& filepath)override
	{
		if (_loaded) return true;

		// 使用OpenCV加载图像
		_imageData = cv::imread(filepath, cv::IMREAD_UNCHANGED);
		if (_imageData.empty()) {
			return false;
		}

		// 转换为RGBA格式（如果需要）
		if (_imageData.channels() == 3) {
			cv::cvtColor(_imageData, _imageData, cv::COLOR_BGR2RGBA);
		}
		else if (_imageData.channels() == 1) {
			cv::cvtColor(_imageData, _imageData, cv::COLOR_GRAY2RGBA);
		}

		_width = _imageData.cols;
		_height = _imageData.rows;
		_channels = _imageData.channels();

		IAYResource::load(filepath);

		_loaded = true;
		return true;
	}
	virtual bool unload()override
	{
		if (!_loaded) return true;
		std::lock_guard<std::mutex> lock(_dataMutex);
		releaseData();
		_loaded = false;
		return true;
	}
	virtual bool reload(const std::string& filepath)override
	{
		IAYResource::reload(filepath);

		return true;
	}
	virtual size_t sizeInBytes()override
	{
		return _imageData.total() * _imageData.elemSize();
	}

private:
	void releaseData(){
		_imageData.release();
		_width = _height = _channels = 0;
	}

	int _width;
	int _height;
	int _channels;
	cv::Mat _imageData;
	mutable std::mutex _dataMutex;
	TextureFormat _format;
};

REGISTER_RESOURCE_CLASS(AYTexture, 0)
//REGISTER_RESOURCE_CLASS(AYTexture, 1, unsigned int)
