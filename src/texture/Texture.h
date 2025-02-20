#pragma once
#include "core/core.h"
#include <string>
class Texture
{
public:
	Texture(const std::string& img_path, unsigned int unitOffset);
	~Texture();

	void bindTexture();

	Texture() = delete;
private:
	GLuint texture;
	unsigned int width;
	unsigned int height;
	unsigned int unitOffset;
};