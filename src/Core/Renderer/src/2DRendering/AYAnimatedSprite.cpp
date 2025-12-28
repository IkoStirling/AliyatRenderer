#include "2DRendering/AYAnimatedSprite.h"

namespace ayt::engine::render
{
    AYAnimatedSprite::AYAnimatedSprite(AYSpriteRenderer* renderer, std::shared_ptr<AYSpriteAtlas> atlas) :
        _renderer(renderer)
    {
        addAtlas(atlas);
    }

    void AYAnimatedSprite::playAnimation(const std::string& name, bool forceRestart)
    {
        auto clip = _findClip(name);
        if (clip)
            _controller.play(clip, forceRestart);
    }

    void AYAnimatedSprite::queueAnimation(const std::string& name)
    {
        auto clip = _findClip(name);
        if (clip)
            _controller.queueAnimation(clip);
    }

    bool AYAnimatedSprite::isCurrentAnimationDone() const
    {
        return _controller.isCurrentAnimationDone();
    }


    void AYAnimatedSprite::update(float deltaTime)
    {
        _controller.update(deltaTime);
    }

    void AYAnimatedSprite::render(
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

    void AYAnimatedSprite::addAtlas(std::shared_ptr<AYSpriteAtlas> atlas)
    {
        if (atlas)
            _atlases.push_back(atlas);
    }

    AYAnimationController& AYAnimatedSprite::getController()
    {
        return _controller;
    }

    std::shared_ptr<AYAnimationClip> AYAnimatedSprite::_findClip(const std::string& name)
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