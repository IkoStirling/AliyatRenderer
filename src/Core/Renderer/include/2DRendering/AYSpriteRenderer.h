#pragma once
#include "AYRenderDevice.h"
#include "STTransform.h"
#include <memory>
namespace ayt::engine::render
{
    class Renderer;

    class SpriteRenderer {
    public:
        SpriteRenderer(RenderDevice* device, Renderer* renderer);
        ~SpriteRenderer();
        void shutdown();

        void drawSprite(GLuint texture,
            const math::Transform& transform,
            const math::Vector2& uvOffset,
            const math::Vector2& uvSize,
            const math::Vector2& size = math::Vector2(1.0f),
            const math::Vector4& color = math::Vector4(1.0f),
            bool flipHorizontal = false,
            bool flipVertical = false,
            const math::Vector2& origin = math::Vector2(0.5f)
        );

        void drawSpriteFromAtlas(GLuint texture,
            const math::Transform& transform,
            const math::Vector2& uvOffset,
            const math::Vector2& uvSize,
            const math::Vector2& size = math::Vector2(1.0f),
            const math::Vector4& color = math::Vector4(1.0f),
            bool flipHorizontal = false,
            bool flipVertical = false,
            const math::Vector2& origin = math::Vector2(0.5f)
        );

        void drawSprite3D(GLuint texture,
            const math::Transform& transform,
            const math::Vector2& uvOffset = math::Vector2(0.f),
            const math::Vector2& uvSize = math::Vector2(1.f),
            const math::Vector3& size = math::Vector3(1.0f),
            const math::Vector4& color = math::Vector4(1.0f),
            bool flipHorizontal = false,
            bool flipVertical = false,
            const math::Vector3& origin = math::Vector3(0.5f)
        );

        void drawSpriteFromAtlas3D(GLuint texture,
            const math::Transform& transform,
            const math::Vector2& uvOffset,
            const math::Vector2& uvSize,
            const math::Vector3& size = math::Vector3(1.0f),
            const math::Vector4& color = math::Vector4(1.0f),
            bool flipHorizontal = false,
            bool flipVertical = false,
            const math::Vector3& origin = math::Vector3(0.5f)
        );


    private:
        RenderDevice* _device;
        Renderer* _renderer;

        void _initBuffers();

        GLuint _vao = 0;
        GLuint _vbo = 0;

    private:
        void _loadSpriteRendererConfigINI();
        void _saveSpriteRendererConfigINI();
        GLuint _getBaseShader(bool reload = false);
        GLuint _getAtlasShader(bool reload = false);

        config::Configer _config;
        std::string _configPath;
        std::string _baseShader;
        std::string _atlasShader;
        std::string _baseVertex;
        std::string _atlasVertex;
        std::string _baseFragment;
        std::string _atlasFragment;
    };
}