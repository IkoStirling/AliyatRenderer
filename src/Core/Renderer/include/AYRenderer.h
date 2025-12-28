#pragma once
#include "AYRenderDevice.h"
#include "BaseRendering/AYRenderContext.h"
#include "BaseRendering/AYCoreRenderer.h"
#include "BaseRendering/AYUIRenderer.h"
#include "BaseRendering/Light/AYLightManager.h"
#include "BaseRendering/Camera/AYCameraSystem.h"
#include "2DRendering/AYFontRenderer.h"
#include "2DRendering/AYSpriteRenderer.h"
#include "3DRendering/AYSkyboxRenderer.h"
#include "AYMaterialManager.h"
#include <memory>
namespace ayt::engine::render
{
    class Renderer
    {
    public:
        explicit Renderer(RenderDevice* device);
        void shutdown();
        ~Renderer();

        void clearScreen(float r, float g, float b, float a);
        void drawFullscreenQuad();

        RenderContext& getRenderContext() { return _context; }
        SpriteRenderer* getSpriteRenderer() { return _spriteRenderer.get(); }
        FontRenderer* getFontRenderer() { return _fontRenderer.get(); }
        CoreRenderer* getCoreRenderer() { return _coreRenderer.get(); }
        UIRenderer* getUIRenderer() { return _uiRenderer.get(); }
        SkyboxRenderer* getSkyboxRenderer() { return _skyboxRenderer.get(); }

        MaterialManager* getMaterialManager() { return GET_CAST_MODULE(MaterialManager, "MaterialManager").get(); }
        LightManager* getLightManager() { return _lightManager.get(); }
        CameraSystem* getCameraSystem() { return _cameraSystem.get(); }

    private:
        void _initFontRenderer();
        void _initSpriteRenderer();
        void _initCoreRenderer();
        void _initUIRenderer();
        void _initSkyboxRenderer();
        void _initMaterialManager();
        void _initLightManager();
        void _initCameraSystem();

        RenderDevice* _device;
        RenderContext _context;

        std::unique_ptr<FontRenderer> _fontRenderer;
        std::unique_ptr<SpriteRenderer> _spriteRenderer;
        std::unique_ptr<CoreRenderer> _coreRenderer;
        std::unique_ptr<UIRenderer> _uiRenderer;
        std::unique_ptr<SkyboxRenderer> _skyboxRenderer;

        std::unique_ptr<LightManager> _lightManager;
        std::unique_ptr<CameraSystem> _cameraSystem;

        GLuint _screenVAO = 0;
    };
}