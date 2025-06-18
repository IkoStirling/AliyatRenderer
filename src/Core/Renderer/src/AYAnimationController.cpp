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

        //�������ǰ����֡���Ѳ���
        if (_currentFrame >= _currentClip->frameCount()) 
        {
            if (_currentClip->isLoop) 
            {
                _currentFrame = 0;
            }
            else 
            {
                // ��ǰ�����������
                if (!_animationQueue.empty()) 
                {
                    // �Ӷ���ȡ����һ������
                    _currentClip = _animationQueue.front();
                    _animationQueue.pop();
                    _currentFrame = 0;
                }
                else 
                {
                    // �����ж���������ʱ����ص������ʱѭ�������򲻻ᴥ��
                    if(_endCallback)
                        _endCallback();
                    // û�д����Ŷ�����ֹͣ
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
    //��������ָ��,����յ�ǰ����
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
