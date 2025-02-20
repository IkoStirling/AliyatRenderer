#include "Texture.h"
#include "opencv2/opencv.hpp"


Texture::Texture(const std::string& img_path, unsigned int unitOffset):
	texture(0),
	width(0),
	height(0),
	unitOffset(0)
{
	cv::Mat image = cv::imread(img_path, cv::IMREAD_UNCHANGED);
	if (image.empty())
	{
		std::cerr << "image load error" << std::endl;
		return;
	}
	cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
	cv::flip(image, image, 0);

	width = image.cols;
	height = image.rows;

	glGenTextures(1, &texture);
	bindTexture();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
}

Texture::~Texture()
{
	if (texture != 0)
		glDeleteTextures(1,&texture);
}

void Texture::bindTexture()
{
	glActiveTexture(GL_TEXTURE0 + unitOffset);
	glBindTexture(GL_TEXTURE_2D, texture);
}
