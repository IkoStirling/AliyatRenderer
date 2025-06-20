#pragma once
#include "AYAnimationClip.h"
#include <queue>
#include <functional>

class AYAnimationController {
public:
    enum class State {
        Idle,
        Playing,
        Paused
    };


    void play(std::shared_ptr<AYAnimationClip> clip, bool forceRestart = false); //ǿ�����²���
    void queueAnimation(std::shared_ptr<AYAnimationClip> clip);
    void update(float deltaTime);
    void pause();
    void resume();
    void stop();

    using AnimationCallback = std::function<void()>;

    void setAnimationEndCallback(AnimationCallback callback) {
        _endCallback = callback;
    }

    const AYAnimationFrame& getCurrentFrame() const;
    bool isPlaying() const;
    bool isCurrentAnimationDone() const;    //ѭ����������Ϊ�ɴ�ϵģ��˴�����true
    State getState() const { return _state; }

private:
    std::shared_ptr<AYAnimationClip> _currentClip;
    std::queue<std::shared_ptr<AYAnimationClip>> _animationQueue;
    size_t _currentFrame = 0;
    float _frameTimer = 0.0f;
    State _state = State::Idle;

    AnimationCallback _endCallback;
};