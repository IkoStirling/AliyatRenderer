#include "2DRendering/AYAnimatedSprite.h"

namespace ayt::engine::render
{
    AnimatedSprite::AnimatedSprite(SpriteRenderer* renderer, std::shared_ptr<SpriteAtlas> atlas) :
        _renderer(renderer)
    {
        addAtlas(atlas);
    }

    void AnimatedSprite::playAnimation(const std::string& name, bool forceRestart)
    {
        auto clip = _findClip(name);
        if (clip)
            _controller.play(clip, forceRestart);
    }

    void AnimatedSprite::queueAnimation(const std::string& name)
    {
        auto clip = _findClip(name);
        if (clip)
            _controller.queueAnimation(clip);
    }

    bool AnimatedSprite::isCurrentAnimationDone() const
    {
        return _controller.isCurrentAnimationDone();
    }


    void AnimatedSprite::update(float deltaTime)
    {
        _controller.update(deltaTime);
    }

    void AnimatedSprite::render(
        const math::Transform& transform,
        const math::Vector3& size,
        const math::Vector4& color,
        bool flipHorizontal,
        bool flipVertical,
        const math::Vector3& origin
    )
    {
        if (!_controller.isPlaying()) return;

        const auto& frame = _controller.getCurrentFrame();
        _renderer->drawSpriteFromAtlas3D(
            _atlases[_curAtlas]->getTexture(),
            transform,
            frame.uvOffset,
            frame.uvSize,
            size,
            color,
            flipHorizontal,
            flipVertical,
            origin
        );
    }

    void AnimatedSprite::addAtlas(std::shared_ptr<SpriteAtlas> atlas)
    {
        if (atlas)
            _atlases.push_back(atlas);
    }

    AnimationController& AnimatedSprite::getController()
    {
        return _controller;
    }

    std::shared_ptr<AnimationClip> AnimatedSprite::_findClip(const std::string& name)
    {
        for (int i = 0; i < _atlases.size(); i++)
        {
            if (!_atlases[i])
                continue;
            if (_atlases[i]->hasAnimation(name))
            {
                auto clip = _atlases[i]->getAnimation(name);
                _curAtlas = i; //用于索引纹理ID
                return clip;
            }
        }
        return nullptr;
    }


}