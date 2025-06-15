#pragma once
#include "ECRendererDependence.h"

class AYRenderDevice {
public:
    bool init(int width, int height);
    void shutdown();

    GLFWwindow* getWindow();

    GLuint createVertexBuffer(const void* data, size_t size);
    GLuint createIndexBuffer(const void* data, size_t size);
    GLuint createVertexArray();
    GLuint createTexture2D(const uint8_t* pixels, int width, int height, int channels = 4);
    GLuint createFontTexture(const uint8_t* pixels, int width, int height);
    GLuint createShaderProgram(const char* vtx_src, const char* frag_src);
private:
    std::atomic<GLFWwindow*> _window {nullptr};
};
