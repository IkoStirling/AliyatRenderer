#pragma once
#include "IAYResource.h"
#include "AYResourceRegistry.h"
#include "AYMemoryPool.h"
#include <iostream>
#include <atomic>
#include "opencv2/opencv.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"


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
		_textureID(0),
		_width(0),
		_height(0),
		_channels(0)
	{
	}
	AYTexture(unsigned int format) :
		_format((TextureFormat)(format)),
		_textureID(0),
		_width(0),
		_height(0),
		_channels(0) 
	{}

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
	void bind(GLuint textureUnit) const {
		GLuint id = _textureID.load(std::memory_order_acquire);
		if (id != 0) {
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, id);
		}
	}

	// ��ResourceManager���ã��ϴ�������ͷ�CPU������
	void uploadToGPU(GLuint textureID) {
		if (_textureID != 0) return;

		std::lock_guard<std::mutex> lock(_dataMutex);
		if (_imageData.empty()) return;

		GLuint tempID = 0;
		glGenTextures(1, &tempID); 
		_textureID.store(tempID, std::memory_order_release);
		glBindTexture(GL_TEXTURE_2D, _textureID);

		// �Զ�ȷ���ڲ���ʽ
		GLenum internalFormat = GL_RGBA8;
		GLenum dataFormat = GL_RGBA;
		if (_channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (_channels == 1) {
			internalFormat = GL_R8;
			dataFormat = GL_RED;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
			_width, _height, 0,
			dataFormat, GL_UNSIGNED_BYTE, _imageData.data);

		// �Զ�����mipmap����ѡ��
		glGenerateMipmap(GL_TEXTURE_2D);

		// ����Ĭ�ϲ���
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}

private:
	void releaseData() {
		GLuint id = _textureID.exchange(0);
		if (id != 0) {
			glDeleteTextures(1, &id); // �첽�ͷ���ȷ�������Ĵ���
		}
		_imageData.release();
		_width = _height = _channels = 0;
	}

	std::atomic<GLuint> _textureID{ 0 };  // OpenGL����ID
	int _width;
	int _height;
	int _channels;
	cv::Mat _imageData;
	mutable std::mutex _dataMutex;
	TextureFormat _format;
};

REGISTER_RESOURCE_CLASS(AYTexture, 0)
//REGISTER_RESOURCE_CLASS(AYTexture, 1, unsigned int)
