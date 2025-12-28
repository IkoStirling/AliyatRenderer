#include "AYRenderer.h"
namespace ayt::engine::render
{
    Renderer::Renderer(RenderDevice* device) :
        _device(device)
    {
        _initFontRenderer();
        _initSpriteRenderer();
        _initCoreRenderer();
        _initUIRenderer();
        _initSkyboxRenderer();
        _initMaterialManager();
        _initLightManager();
        _initCameraSystem();
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::shutdown()
    {
        _fontRenderer->shutdown();
        _spriteRenderer->shutdown();
        _coreRenderer->shutdown();
        _uiRenderer->shutdown();
        _skyboxRenderer->shutdown();
        _cameraSystem->shutdown();
    }

    void Renderer::clearScreen(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::drawFullscreenQuad()
    {
        if (_screenVAO == 0) {
            // 初始化全屏四边形（用于后期处理）
            float vertices[] = {
                -1, -1, 0, 0,
                 1, -1, 1, 0,
                 1,  1, 1, 1,
                -1,  1, 0, 1
            };

            GLuint vbo;
            _device->createVertexBuffer(vertices, sizeof(vertices));

            glGenVertexArrays(1, &_screenVAO);
            glBindVertexArray(_screenVAO);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray(0); // position
            glEnableVertexAttribArray(1); // uv
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        }

        glBindVertexArray(_screenVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void Renderer::_initFontRenderer()
    {
        _fontRenderer = std::make_unique<FontRenderer>(_device, this);
        _fontRenderer->loadFont("C:/Windows/Fonts/msyh.ttc", 24);
    }

    void Renderer::_initSpriteRenderer()
    {
        _spriteRenderer = std::make_unique<SpriteRenderer>(_device, this);
    }

    void Renderer::_initCoreRenderer()
    {
        _coreRenderer = std::make_unique<CoreRenderer>(_device, this);
    }

    void Renderer::_initUIRenderer()
    {
        _uiRenderer = std::make_unique<UIRenderer>(_device, this);
    }

    void Renderer::_initSkyboxRenderer()
    {
        _skyboxRenderer = std::make_unique<SkyboxRenderer>(_device, this);
    }

    void Renderer::_initMaterialManager()
    {
    }

    void Renderer::_initLightManager()
    {
        _lightManager = std::make_unique<LightManager>(_device, this);
    }

    void Renderer::_initCameraSystem()
    {
        _cameraSystem = std::make_unique<CameraSystem>();
    }
}