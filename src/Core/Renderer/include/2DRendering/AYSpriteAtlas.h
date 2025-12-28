#pragma once
#include "AYAnimationClip.h"
#include <unordered_map>
#include <memory>
#include "GLFW/glfw3.h"
namespace ayt::engine::render
{
    /*
        精灵图集，一个精灵图集可以包含多个动画切片，精灵图集来源单张纹理
    */
    class AYSpriteAtlas {
    public:
        AYSpriteAtlas(GLuint textureId,
            const math::Vector2& spriteSize,
            const math::Vector2& atlasSize);

        bool addAnimation(const std::string& name,
            std::shared_ptr<AYAnimationClip> clip);

        std::shared_ptr<AYAnimationClip> getAnimation(const std::string& name) const;
        GLuint getTexture() const { return _texture; }
        bool hasAnimation(const std::string& name) const { return _animations.find(name) != _animations.end(); }

    private:
        GLuint _texture;
        math::Vector2 _spriteSize;
        math::Vector2 _atlasSize;
        std::unordered_map<std::string, std::shared_ptr<AYAnimationClip>> _animations;
    };
}