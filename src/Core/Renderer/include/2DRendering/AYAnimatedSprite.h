#pragma once
#include "AYSpriteRenderer.h"
#include "AYAnimationController.h"
#include "AYSpriteAtlas.h"

class AYAnimatedSprite {
public:
    /*
        要求默认至少拥有一个精灵图集
    */
    AYAnimatedSprite(AYSpriteRenderer* renderer, std::shared_ptr<AYSpriteAtlas> atlas);

    void playAnimation(const std::string& name, bool forceRestart = false);
    void queueAnimation(const std::string& name);
    bool isCurrentAnimationDone() const;

    void update(float deltaTime);
    void render(const glm::vec2& position,
        const glm::vec2& size,
        float rotation = 0.0f,
        const glm::vec4& color = glm::vec4(1.0f),
        bool flipHorizontal = false,
        bool flipVertical = false,
        const glm::vec2& origin = glm::vec2(0.5f));

    void addAtlas(std::shared_ptr<AYSpriteAtlas> atlas);

    AYAnimationController& getController();

private:
    std::shared_ptr<AYAnimationClip> _findClip(const std::string& name);
    AYSpriteRenderer* _renderer;
    AYAnimationController _controller;
    std::vector<std::shared_ptr<AYSpriteAtlas>> _atlases;
    int _curAtlas = 0; //默认使用第一个精灵图集
};
