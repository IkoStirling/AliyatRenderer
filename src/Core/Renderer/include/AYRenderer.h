#pragma once
#include "AYRenderDevice.h"
#include "AYFontRenderer.h"
#include "AYSpriteRenderer.h"

class AYRenderer
{
public:
    explicit AYRenderer(AYRenderDevice* device);
    ~AYRenderer();

    void clearScreen(float r, float g, float b, float a);
    void drawFullscreenQuad();
    void setViewport(int x, int y, int width, int height);
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);

    AYSpriteRenderer* getSpriteRenderer();
private:
    void _initFontRenderer();
    void _initSpriteRenderer();

    AYRenderDevice* _device;
    AYFontRenderer* _fontRenderer;
    AYSpriteRenderer* _spriteRenderer;

    GLuint _screenVAO = 0;

};