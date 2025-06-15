#pragma once
#include "AYRenderDevice.h"

class AYRenderer
{
public:
    explicit AYRenderer(AYRenderDevice* device) : _device(device) {}

    void clearScreen(float r, float g, float b, float a);
    void drawFullscreenQuad();
    void setViewport(int x, int y, int width, int height);

private:
    AYRenderDevice* _device;
    GLuint _screenVAO = 0;
};