#pragma once
#include "AYSpriteRenderer.h"
#include "AYAnimationController.h"
#include "AYSpriteAtlas.h"
namespace ayt::engine::render
{
    class AnimatedSprite {
    public:
        /*
            要求默认至少拥有一个精灵图集
        */
        AnimatedSprite(SpriteRenderer* renderer, std::shared_ptr<SpriteAtlas> atlas);

        void playAnimation(const std::string& name, bool forceRestart = false);
        void queueAnimation(const std::string& name);
        bool isCurrentAnimationDone() const;

        void update(float deltaTime);
        void render(
            const math::Transform& transform,
            const math::Vector3& size = math::Vector3(1.0f),
            const math::Vector4& color = math::Vector4(1.0f),
            bool flipHorizontal = false,
            bool flipVertical = false,
            const math::Vector3& origin = math::Vector3(0.5f)
        );

        void addAtlas(std::shared_ptr<SpriteAtlas> atlas);

        AnimationController& getController();

    private:
        std::shared_ptr<AnimationClip> _findClip(const std::string& name);
        SpriteRenderer* _renderer;
        AnimationController _controller;
        std::vector<std::shared_ptr<SpriteAtlas>> _atlases;
        int _curAtlas = 0; //默认使用第一个精灵图集
    };
}