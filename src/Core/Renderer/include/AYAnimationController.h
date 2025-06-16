#pragma once
#include "AYAnimationClip.h"

class AYAnimationController {
public:
    /*
        非强制播放，如果当前正在播放该动画，则没有效果
    */
    void play(std::shared_ptr<AYAnimationClip> clip, bool forceRestart = false);

    /*
        逻辑更新，用于控制当前帧位置
    */
    void update(float deltaTime);

    /*
        停止当前动画
    */
    void stop();

    const AYAnimationFrame& getCurrentFrame() const;
    bool isPlaying() const;

private:
    std::shared_ptr<AYAnimationClip> _currentClip;
    size_t _currentFrame = 0;
    float _frameTimer = 0.0f;
};