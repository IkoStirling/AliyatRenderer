#include "SkyBoxTexture.h"

SkyBoxTexture::SkyBoxTexture(const std::string& img_path, unsigned int unitOffset) :
	Texture(img_path, unitOffset)
{

}

SkyBoxTexture::~SkyBoxTexture()
{
}

void SkyBoxTexture::defTextureParams()
{
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (imageData) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

GLuint SkyBoxTexture::createCubemap()
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // 为每个面分配内存
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1080, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

void SkyBoxTexture::renderToCubemap(GLuint equirectangularTexture, GLuint cubemapTexture)
{
    // 创建帧缓冲对象（FBO）
    GLuint framebuffer, rbo;
    glGenFramebuffers(1, &framebuffer);
    glGenRenderbuffers(1, &rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1080, 1080);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // 设置视口
    glViewport(0, 0, 1080, 1080);

    // 渲染到立方体贴图的每个面
    for (unsigned int i = 0; i < 6; i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapTexture, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 渲染代码（使用着色器将等距柱状投影映射到当前面）
        //renderCubeFace(i, equirectangularTexture);
    }

    // 清理
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &rbo);
}

GLuint SkyBoxTexture::getCubemapTextureID()
{
    if (cubemapTexture == 0)
    {
        cubemapTexture = createCubemap();
        renderToCubemap(texture, cubemapTexture);
    }
    return cubemapTexture;
}
