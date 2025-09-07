#include "AYRenderer.h"

AYRenderer::AYRenderer(AYRenderDevice* device) : 
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

AYRenderer::~AYRenderer()
{
}

void AYRenderer::shutdown()
{
    _fontRenderer->shutdown();
    _spriteRenderer->shutdown();
    _coreRenderer->shutdown();
    _uiRenderer->shutdown();
    _skyboxRenderer->shutdown();
    _cameraSystem->shutdown();
}

void AYRenderer::clearScreen(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void AYRenderer::drawFullscreenQuad()
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

void AYRenderer::_initFontRenderer()
{
    _fontRenderer = std::make_unique<AYFontRenderer>(_device, this);
    _fontRenderer->loadFont("C:/Windows/Fonts/msyh.ttc", 24);
    _fontRenderer->saveAtlas();
}

void AYRenderer::_initSpriteRenderer()
{
    _spriteRenderer = std::make_unique<AYSpriteRenderer>(_device, this);
}

void AYRenderer::_initCoreRenderer()
{
    _coreRenderer = std::make_unique<AYCoreRenderer>(_device, this);
}

void AYRenderer::_initUIRenderer()
{
    _uiRenderer = std::make_unique<AYUIRenderer>(_device, this);
}

void AYRenderer::_initSkyboxRenderer()
{
    _skyboxRenderer = std::make_unique<AYSkyboxRenderer>(_device, this);
}

void AYRenderer::_initMaterialManager()
{
}

void AYRenderer::_initLightManager()
{
    _lightManager = std::make_unique<AYLightManager>(_device, this);
}

void AYRenderer::_initCameraSystem()
{
    _cameraSystem = std::make_unique<AYCameraSystem>();
}