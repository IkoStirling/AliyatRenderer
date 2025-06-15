#include "AYRenderer.h"

AYRenderer::AYRenderer(AYRenderDevice* device) : 
    _device(device)
{
    _initFontRenderer();
    _initSpriteRenderer();
}

AYRenderer::~AYRenderer()
{
    delete _fontRenderer;
    delete _spriteRenderer;
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

void AYRenderer::setViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
    _spriteRenderer->setViewportSize(width, height);
}

void AYRenderer::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color)
{
    _fontRenderer->renderText(text, x, y, scale, color);
}

AYSpriteRenderer* AYRenderer::getSpriteRenderer()
{
    return _spriteRenderer;
}

void AYRenderer::_initFontRenderer()
{
    
    _fontRenderer = new AYFontRenderer(_device);
    _fontRenderer->loadFont("C:/Windows/Fonts/msyh.ttc", 24);
}

void AYRenderer::_initSpriteRenderer()
{
    _spriteRenderer = new AYSpriteRenderer(_device);
}
