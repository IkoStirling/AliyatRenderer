#pragma once
#include "AYRenderDevice.h"
#include "BaseRendering/AYRenderContext.h"
#include "2DRendering/AYFontRenderer.h"
#include "2DRendering/AYSpriteRenderer.h"
#include "BaseRendering/AYCoreRenderer.h"
#include "AYMaterialManager.h"
#include "BaseRendering/Light/AYLightManager.h"
#include "BaseRendering/Camera/AYCameraSystem.h"
#include "3DRendering/AYSkyboxRenderer.h"
#include <memory>

class AYRenderer
{
public:
    explicit AYRenderer(AYRenderDevice* device);
    void shutdown();
    ~AYRenderer();

    void clearScreen(float r, float g, float b, float a);
    void drawFullscreenQuad();

    AYRenderContext& getRenderContext() { return _context; }
    AYSpriteRenderer* getSpriteRenderer() { return _spriteRenderer.get(); }
    AYFontRenderer* getFontRenderer() { return _fontRenderer.get(); }
    AYCoreRenderer* getCoreRenderer() { return _coreRenderer.get(); }
    AYSkyboxRenderer* getSkyboxRenderer() { return _skyboxRenderer.get(); }

    AYMaterialManager* getMaterialManager() { return GET_CAST_MODULE(AYMaterialManager,"MaterialManager").get(); }
    AYLightManager* getLightManager() { return _lightManager.get(); }
    AYCameraSystem* getCameraSystem() { return _cameraSystem.get(); }

private:
    void _initFontRenderer();
    void _initSpriteRenderer();
    void _initCoreRenderer();
    void _initSkyboxRenderer();
    void _initMaterialManager();
    void _initLightManager();
    void _initCameraSystem();

    AYRenderDevice* _device;
    AYRenderContext _context;

    std::unique_ptr<AYFontRenderer> _fontRenderer;
    std::unique_ptr<AYSpriteRenderer> _spriteRenderer;
    std::unique_ptr<AYCoreRenderer> _coreRenderer;
    std::unique_ptr<AYSkyboxRenderer> _skyboxRenderer;

    std::unique_ptr<AYLightManager> _lightManager;
    std::unique_ptr<AYCameraSystem> _cameraSystem;

    GLuint _screenVAO = 0;
};