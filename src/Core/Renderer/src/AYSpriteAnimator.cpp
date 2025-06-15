#include "AYSpriteAnimator.h"

AYSpriteAnimator::AYSpriteAnimator(const glm::vec2& spriteSize, const glm::vec2& atlasSize)
    : _spriteSize(spriteSize), _atlasSize(atlasSize) {
}

void AYSpriteAnimator::addAnimation(const std::string& name,
    const std::vector<AnimationFrame>& frames,
    bool loop) {
    Animation anim;
    anim.name = name;
    anim.frames = frames;
    anim.loop = loop;
    _animations[name] = anim;
}

void AYSpriteAnimator::play(const std::string& name, bool forceRestart) {
    auto it = _animations.find(name);
    if (it == _animations.end()) return;

    if (forceRestart || _currentAnimation != &it->second) {
        _currentAnimation = &it->second;
        _currentFrameIndex = 0;
        _frameTimer = 0.0f;
    }
}

void AYSpriteAnimator::update(float deltaTime) {
    if (!_currentAnimation) return;

    _frameTimer += deltaTime;
    const auto& frame = _currentAnimation->frames[_currentFrameIndex];

    if (_frameTimer >= frame.duration) {
        _frameTimer = 0.0f;
        _currentFrameIndex++;

        if (_currentFrameIndex >= _currentAnimation->frames.size()) {
            if (_currentAnimation->loop) {
                _currentFrameIndex = 0;
            }
            else {
                stop();
            }
        }
    }
}

void AYSpriteAnimator::stop() {
    _currentAnimation = nullptr;
    _currentFrameIndex = 0;
    _frameTimer = 0.0f;
}

const AYSpriteAnimator::AnimationFrame& AYSpriteAnimator::getCurrentFrame() const {
    static AnimationFrame defaultFrame;
    if (!_currentAnimation || _currentAnimation->frames.empty())
        return defaultFrame;

    return _currentAnimation->frames[_currentFrameIndex];
}