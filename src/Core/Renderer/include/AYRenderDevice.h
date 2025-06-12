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
    GLuint createTexture2D(const uint8_t* pixels, int width, int height);

private:
    GLFWwindow* _window = nullptr;
};
