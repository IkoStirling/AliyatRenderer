#pragma once
#include "AYRenderDevice.h"
#include "BaseRendering/AYRenderContext.h"
#include "2DRendering/AYFontRenderer.h"
#include "2DRendering/AYSpriteRenderer.h"
#include "BaseRendering/AYCoreRenderer.h"
#include "3DRendering/AYSkyboxRenderer.h"
#include <memory>

class AYRenderer
{
public:
    explicit AYRenderer(AYRenderDevice* device);
    ~AYRenderer();

    void clearScreen(float r, float g, float b, float a);
    void drawFullscreenQuad();
    void setViewport(int x, int y, int width, int height);
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);

    AYRenderContext& getRenderContext() { return _context; }
    AYSpriteRenderer* getSpriteRenderer();
    AYFontRenderer* getFontRenderer();
    AYCoreRenderer* getCoreRenderer();
    AYSkyboxRenderer* getSkyboxRenderer();

private:
    void _initFontRenderer();
    void _initSpriteRenderer();
    void _initCoreRenderer();
    void _initSkyboxRenderer();

    AYRenderDevice* _device;
    AYRenderContext _context;

    std::unique_ptr<AYFontRenderer> _fontRenderer;
    std::unique_ptr<AYSpriteRenderer> _spriteRenderer;
    std::unique_ptr<AYCoreRenderer> _coreRenderer;
    std::unique_ptr<AYSkyboxRenderer> _skyboxRenderer;

    GLuint _screenVAO = 0;
};