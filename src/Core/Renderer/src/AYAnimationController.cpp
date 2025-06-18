#include "AYAnimationController.h"

void AYAnimationController::play(std::shared_ptr<AYAnimationClip> clip, bool forceRestart) 
{
    while (!_animationQueue.empty()) _animationQueue.pop();

    if (!forceRestart && _currentClip == clip) return;

    _currentClip = clip;
    _currentFrame = 0;
    _frameTimer = 0.0f;
    _state = State::Playing;
}

void AYAnimationController::queueAnimation(std::shared_ptr<AYAnimationClip> clip)
{
    if (!_currentClip) {
        play(clip);
    }
    else {
        _animationQueue.push(clip);
    }
}

void AYAnimationController::update(float deltaTime) {
    if (!_currentClip || _state != State::Playing) return;

    _frameTimer += deltaTime;
    const auto& frame = _currentClip->getFrame(_currentFrame);

    if (_frameTimer >= frame.duration) {
        _frameTimer = 0.0f;
        _currentFrame++;

        //这里代表当前动画帧数已播完
        if (_currentFrame >= _currentClip->frameCount()) 
        {
            if (_currentClip->isLoop) 
            {
                _currentFrame = 0;
            }
            else 
            {
                // 当前动画播放完成
                if (!_animationQueue.empty()) 
                {
                    // 从队列取出下一个动画
                    _currentClip = _animationQueue.front();
                    _animationQueue.pop();
                    _currentFrame = 0;
                }
                else 
                {
                    // 当所有动画播放完时处理回调，如果时循环动画则不会触发
                    if(_endCallback)
                        _endCallback();
                    // 没有待播放动画，停止
                    _state = State::Idle;
                    _currentClip.reset();
                    return;
                }
            }
        }
    }
}

void AYAnimationController::pause()
{
    _state = State::Paused;
}

void AYAnimationController::resume()
{
    _state = State::Playing;
}

void AYAnimationController::stop() {
    _state = State::Idle;
    //重置智能指针,即清空当前对象
    _currentClip.reset();
    std::queue<std::shared_ptr<AYAnimationClip>> queue;
    _animationQueue.swap(queue);
}

const AYAnimationFrame& AYAnimationController::getCurrentFrame() const {
    static AYAnimationFrame defaultFrame;
    return _currentClip ? _currentClip->getFrame(_currentFrame) : defaultFrame;
}

bool AYAnimationController::isPlaying() const {
    return _currentClip != nullptr;
}

bool AYAnimationController::isCurrentAnimationDone() const
{
    if (_currentClip && _currentClip->isLoop)
        return true;
    return _state == State::Idle && !_currentClip;
}
