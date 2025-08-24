#pragma once
#include "AYRenderDevice.h"
#include "STTransform.h"
#include <glm/glm.hpp>
#include <memory>

class AYRenderer;

class AYSpriteRenderer {
public:
    AYSpriteRenderer(AYRenderDevice* device, AYRenderer* renderer);
    ~AYSpriteRenderer();
    void shutdown();

    void drawSprite(GLuint texture,
        const STTransform& transform,
        const glm::vec2& uvOffset,
        const glm::vec2& uvSize,
        const glm::vec2& size = glm::vec2(1.0f),
        const glm::vec4& color = glm::vec4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const glm::vec2& origin = glm::vec2(0.5f)
        );

    void drawSpriteFromAtlas(GLuint texture,
        const STTransform& transform,
        const glm::vec2& uvOffset,
        const glm::vec2& uvSize,
        const glm::vec2& size = glm::vec2(1.0f),
        const glm::vec4& color = glm::vec4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const glm::vec2& origin = glm::vec2(0.5f)
        );

    void drawSprite3D(GLuint texture,
        const STTransform& transform,
        const glm::vec2& uvOffset = glm::vec2(0.f),
        const glm::vec2& uvSize = glm::vec2(1.f),
        const glm::vec3& size = glm::vec3(1.0f),
        const glm::vec4& color = glm::vec4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const glm::vec3& origin = glm::vec3(0.5f)
        );

    void drawSpriteFromAtlas3D(GLuint texture,
        const STTransform& transform,
        const glm::vec2& uvOffset,
        const glm::vec2& uvSize,
        const glm::vec3& size = glm::vec3(1.0f),
        const glm::vec4& color = glm::vec4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const glm::vec3& origin = glm::vec3(0.5f)
        );


private:
    AYRenderDevice* _device;
    AYRenderer* _renderer;

    void _initBuffers();

    GLuint _vao = 0; 
    GLuint _vbo = 0; 

private:
    void _loadSpriteRendererConfigINI();
    void _saveSpriteRendererConfigINI();
    GLuint _getBaseShader(bool reload = false);
    GLuint _getAtlasShader(bool reload = false);

    AYConfigWrapper _config;
    std::string _configPath;
    std::string _baseShader;
    std::string _atlasShader;
    std::string _baseVertex;
    std::string _atlasVertex;
    std::string _baseFragment;
    std::string _atlasFragment;
};