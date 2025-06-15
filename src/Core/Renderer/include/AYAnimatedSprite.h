#pragma once
#include "AYSpriteAnimator.h"
#include "AYSpriteRenderer.h"

class AYAnimatedSprite {
public:
    AYAnimatedSprite(AYSpriteRenderer* renderer, GLuint texture);

    void update(float deltaTime);
    void render(const glm::vec2& position,
        const glm::vec2& size,
        float rotation = 0.0f,
        const glm::vec4& color = glm::vec4(1.0f),
        const glm::vec2& origin = glm::vec2(0.5f));

    AYSpriteAnimator& getAnimator() { return _animator; }

private:
    AYSpriteRenderer* _renderer;
    GLuint _texture;
    AYSpriteAnimator _animator;
};
