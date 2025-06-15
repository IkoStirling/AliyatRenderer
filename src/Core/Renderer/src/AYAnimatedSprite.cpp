#include "AYAnimatedSprite.h"


AYAnimatedSprite::AYAnimatedSprite(AYSpriteRenderer* renderer, GLuint texture):
    _renderer(renderer),
    _texture(texture) ,
    _animator(glm::vec2(100, 100), glm::vec2(600, 100))
{}

void AYAnimatedSprite::update(float deltaTime) 
{
    _animator.update(deltaTime);
}

void AYAnimatedSprite::render(const glm::vec2& position,
    const glm::vec2& size,
    float rotation,
    const glm::vec4& color,
    const glm::vec2& origin) 
{
    if (!_animator.isPlaying()) return;

    const auto& frame = _animator.getCurrentFrame();
    _renderer->drawSpriteFromAtlas(
        _texture,
        position,
        size,
        frame.uvOffset,
        frame.uvSize,
        rotation,
        color,
        origin
    );
}