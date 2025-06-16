#include "AYAnimatedSprite.h"


AYAnimatedSprite::AYAnimatedSprite(AYSpriteRenderer* renderer, std::shared_ptr<AYSpriteAtlas> atlas):
    _renderer(renderer)
{
    addAtlas(atlas);
}

void AYAnimatedSprite::playAnimation(const std::string& name, bool forceRestart)
{
    for (int i = 0; i < _atlases.size(); i++)
    {
        if (!_atlases[i])
            continue;
        if (_atlases[i]->hasAnimation(name))
        {
            auto clip = _atlases[i]->getAnimation(name);
            _controller.play(clip, forceRestart);
            _curAtlas = i; //用于索引纹理ID
        }
    }
}

void AYAnimatedSprite::update(float deltaTime)
{
    _controller.update(deltaTime);
}

void AYAnimatedSprite::render(const glm::vec2& position,
    const glm::vec2& size,
    float rotation,
    const glm::vec4& color,
    const glm::vec2& origin) 
{
    if (!_controller.isPlaying()) return;

    const auto& frame = _controller.getCurrentFrame();
    _renderer->drawSpriteFromAtlas(
        _atlases[_curAtlas]->getTexture(),
        position,
        size,
        frame.uvOffset,
        frame.uvSize,
        rotation,
        color,
        origin
    );
}

void AYAnimatedSprite::addAtlas(std::shared_ptr<AYSpriteAtlas> atlas)
{
    if (atlas)
        _atlases.push_back(atlas);
}


