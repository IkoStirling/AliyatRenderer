#pragma once
#include "AYRenderDevice.h"
#include "STTransform.h"
#include <memory>

class AYRenderer;

class AYSpriteRenderer {
public:
    AYSpriteRenderer(AYRenderDevice* device, AYRenderer* renderer);
    ~AYSpriteRenderer();
    void shutdown();

    void drawSprite(GLuint texture,
        const STTransform& transform,
        const AYMath::Vector2& uvOffset,
        const AYMath::Vector2& uvSize,
        const AYMath::Vector2& size = AYMath::Vector2(1.0f),
        const AYMath::Vector4& color = AYMath::Vector4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const AYMath::Vector2& origin = AYMath::Vector2(0.5f)
        );

    void drawSpriteFromAtlas(GLuint texture,
        const STTransform& transform,
        const AYMath::Vector2& uvOffset,
        const AYMath::Vector2& uvSize,
        const AYMath::Vector2& size = AYMath::Vector2(1.0f),
        const AYMath::Vector4& color = AYMath::Vector4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const AYMath::Vector2& origin = AYMath::Vector2(0.5f)
        );

    void drawSprite3D(GLuint texture,
        const STTransform& transform,
        const AYMath::Vector2& uvOffset = AYMath::Vector2(0.f),
        const AYMath::Vector2& uvSize = AYMath::Vector2(1.f),
        const AYMath::Vector3& size = AYMath::Vector3(1.0f),
        const AYMath::Vector4& color = AYMath::Vector4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const AYMath::Vector3& origin = AYMath::Vector3(0.5f)
        );

    void drawSpriteFromAtlas3D(GLuint texture,
        const STTransform& transform,
        const AYMath::Vector2& uvOffset,
        const AYMath::Vector2& uvSize,
        const AYMath::Vector3& size = AYMath::Vector3(1.0f),
        const AYMath::Vector4& color = AYMath::Vector4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const AYMath::Vector3& origin = AYMath::Vector3(0.5f)
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