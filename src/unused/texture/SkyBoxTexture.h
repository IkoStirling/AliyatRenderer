#pragma once
#include "core/core.h"
#include "Texture.h"
#include <string>


class SkyBoxTexture : public Texture
{
public:
	SkyBoxTexture(const std::string& img_path, unsigned int unitOffset);
	~SkyBoxTexture();

	virtual void defTextureParams() override;

	GLuint createCubemap();
	void renderToCubemap(GLuint equirectangularTexture, GLuint cubemapTexture);
	GLuint getCubemapTextureID();
	SkyBoxTexture() = delete;

private:
	GLuint cubemapTexture;
};