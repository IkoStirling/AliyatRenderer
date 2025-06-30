#pragma once
#include "AYRenderDevice.h"
#include <glm/glm.hpp>
#include <memory>

class AYRenderer;

class AYSpriteRenderer {
public:
    AYSpriteRenderer(AYRenderDevice* device, AYRenderer* renderer);
    ~AYSpriteRenderer();

    void init();

    /*
        单一精灵图
    */
    void drawSprite(GLuint texture,
        const glm::vec2& position,
        const glm::vec2& size = glm::vec2(1.0f),
        float rotation = 0.0f,
        const glm::vec4& color = glm::vec4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const glm::vec2& origin = glm::vec2(0.5f)
        );

    /*
        精灵图集（动画用）
    */
    void drawSpriteFromAtlas(GLuint texture,
        const glm::vec2& position,
        const glm::vec2& size,
        const glm::vec2& uvOffset,
        const glm::vec2& uvSize,
        float rotation = 0.0f,
        const glm::vec4& color = glm::vec4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const glm::vec2& origin = glm::vec2(0.5f)
        );

    void setViewportSize(int width, int height);

private:
    AYRenderer* _renderer;

    glm::mat4 _prepareModel(const glm::vec2& position,
        const glm::vec2& size,
        float rotation,
        const glm::vec2& origin);

    glm::mat4 _getProjecction();

    void _initShader();
    void _initBuffers();


    AYRenderDevice* _device;
    GLuint _vao = 0; //默认vao（此vao无法绑定vbo）
    GLuint _vbo = 0; //无效vbo  =》表明未初始化
    GLuint _shaderProgram = 0; //无效

    int _viewportWidth = 1920;
    int _viewportHeight = 1080;

    void _initAtlasShader();
    GLuint _atlasShaderProgram;
};