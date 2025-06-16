#pragma once
#include "AYAnimationClip.h"

class AYAnimationController {
public:
    /*
        ��ǿ�Ʋ��ţ������ǰ���ڲ��Ÿö�������û��Ч��
    */
    void play(std::shared_ptr<AYAnimationClip> clip, bool forceRestart = false);

    /*
        �߼����£����ڿ��Ƶ�ǰ֡λ��
    */
    void update(float deltaTime);

    /*
        ֹͣ��ǰ����
    */
    void stop();

    const AYAnimationFrame& getCurrentFrame() const;
    bool isPlaying() const;

private:
    std::shared_ptr<AYAnimationClip> _currentClip;
    size_t _currentFrame = 0;
    float _frameTimer = 0.0f;
};