#include "2DRendering/AYSpriteAtlas.h"

AYSpriteAtlas::AYSpriteAtlas(GLuint textureId,
    const AYMath::Vector2& spriteSize,
    const AYMath::Vector2& atlasSize)
    : _texture(textureId), _spriteSize(spriteSize), _atlasSize(atlasSize) 
{
}

bool AYSpriteAtlas::addAnimation(const std::string& name,
    std::shared_ptr<AYAnimationClip> clip) 
{
    if (hasAnimation(name)) {
        return false;
    }
    _animations[name] = clip;
    return true;
}

std::shared_ptr<AYAnimationClip> AYSpriteAtlas::getAnimation(const std::string& name) const 
{
    return hasAnimation(name) ? _animations.find(name)->second : nullptr;
}