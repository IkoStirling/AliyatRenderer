#pragma once
#include "IAYResource.h"
#include "AYResourceRegistry.h"
#include "AYMemoryPool.h"
#include <iostream>
#include <atomic>
#include "opencv2/opencv.hpp"


class AYTexture : public IAYResource
{
	SUPPORT_MEMORY_POOL(AYTexture)
	DECLARE_RESOURCE_CLASS(AYTexture, "textures")
public:
	using GLuint = unsigned int;
	AYTexture() : _textureID(0), _width(0), _height(0), _channels(0) {}

	virtual ~AYTexture() {
		releaseData();
	}

	// ��ȡ����ID
	GLuint getTextureID() const { return _textureID; }

	// ��ȡ����ߴ�
	int getWidth() const { return _width; }
	int getHeight() const { return _height; }
	int getChannels() const { return _channels; }

	//��ȡOpenCV��ʽ��ͼ������
	const cv::Mat& getImageData() const { return _imageData; }

	const uint8_t* getPixelData() const {
		return _imageData.ptr();
	}

	virtual bool load(const std::string& filepath)override
	{
		if (_loaded) return true;

		// ʹ��OpenCV����ͼ��
		_imageData = cv::imread(filepath, cv::IMREAD_UNCHANGED);
		if (_imageData.empty()) {
			//spdlog::error("Failed to load texture with OpenCV: ", filepath);
			return false;
		}

		// ת��ΪRGBA��ʽ�������Ҫ��
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

		//spdlog::debug("Texture loading info: ", _resourcePath);

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
		//spdlog::debug("Texture reload");

		return true;
	}
	virtual size_t sizeInBytes()override
	{
		return _imageData.total() * _imageData.elemSize();
	}

	// ��ResourceManager���ã��ϴ�������ͷ�CPU������
	void uploadToGPU(GLuint textureID) {
		std::lock_guard<std::mutex> lock(_dataMutex);
		_textureID = textureID;

		// �ϴ������ѡ�������ͷ�CPU����
		// _imageData.release();
	}

private:
	void releaseData() {
		_imageData.release();
		_width = _height = _channels = 0;
		_textureID = 0;
	}

	std::atomic<GLuint> _textureID{ 0 };  // OpenGL����ID
	int _width;
	int _height;
	int _channels;
	cv::Mat _imageData;
	mutable std::mutex _dataMutex;
};

REGISTER_RESOURCE_CLASS(AYTexture)