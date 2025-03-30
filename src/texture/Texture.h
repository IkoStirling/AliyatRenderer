#pragma once
#include "core/core.h"
#include <string>
#include "opencv2/opencv.hpp"

class Texture
{
public:
	Texture(const std::string& img_path, unsigned int unitOffset);
	~Texture();

	virtual void defTextureParams();
	void bindTexture();

	GLuint getTextureID() {
		return texture;
	}

	Texture() = delete;
protected:
	GLuint texture;
	unsigned int width;
	unsigned int height;
	uchar* imageData;
	unsigned int unitOffset;
};