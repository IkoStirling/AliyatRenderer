#include "AYAnimationController.h"

void AYAnimationController::play(std::shared_ptr<AYAnimationClip> clip, bool forceRestart) 
{
    if (!forceRestart && _currentClip == clip) return;

    _currentClip = clip;
    _currentFrame = 0;
    _frameTimer = 0.0f;
}

void AYAnimationController::update(float deltaTime) {
    if (!_currentClip || _currentClip->frameCount() == 0) return;

    _frameTimer += deltaTime;
    const auto& frame = _currentClip->getFrame(_currentFrame);

    if (_frameTimer >= frame.duration) {
        _frameTimer = 0.0f;
        _currentFrame++;

        if (_currentFrame >= _currentClip->frameCount()) {
            if (_currentClip->isLoop) {
                _currentFrame = 0;
            }
            else {
                stop();
            }
        }
    }
}

void AYAnimationController::stop() {
    //重置智能指针,即清空当前对象
    _currentClip.reset();
}

const AYAnimationFrame& AYAnimationController::getCurrentFrame() const {
    static AYAnimationFrame defaultFrame;
    return _currentClip ? _currentClip->getFrame(_currentFrame) : defaultFrame;
}

bool AYAnimationController::isPlaying() const {
    return _currentClip != nullptr;
}