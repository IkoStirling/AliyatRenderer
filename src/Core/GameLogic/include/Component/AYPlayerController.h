#pragma once
#include "IAYComponent.h"
#include "AYMathType.h"

namespace ayt::engine::game
{
    class AYPhysicsComponent;

    class AYPlayerController : public IAYComponent
    {
    public:
        void beginPlay() override;

        void update(float delta_time) override;

        virtual void endPlay();

        void jump();

        void setMoveSpeed(float speed);

        void setRunSpeed(float speed);

        void setJumpForce(float force);

    protected:
        // --------------state machine-------------
        enum class PlayerState {
            Idle,       // 站立静止
            Moving,     // 移动
            Running,    // 奔跑
            Jumping,    // 起跳
            Falling,    // 下落（离开地面后，还未落地）
            Landed,     // 刚落地（只停留一帧，用于触发音效）
            Flying,     // （可选）飞行状态
            Swimming    // （可选）游泳状态
        };
        PlayerState _currentState = PlayerState::Idle;
        PlayerState _lastState = PlayerState::Idle;
        bool _isGrounded;

        float _moveSpeed = 5.0f;
        float _runSpeed = 10.0f;
        float _jumpForce = 10.0f;
        int _maxJumpCount = 2;
        int _currentJumpCount = 0;
        float _jumpCooldownTimer = 0.0f;   // 跳跃冷却计时器
        float _jumpCooldownTime = 0.5f;    // 跳跃冷却时间（单位：秒，比如 100ms）

        bool _shouldJump = false;

    protected:
        AYPhysicsComponent* _physics = nullptr;

        void _updateState();
        
        math::Vector2 _getMovementInput();

        void _updateMovement(const math::Vector2& input, float deltaTime);

        bool _shouldJumpNow();

        void _updateJump();

        void _updateAnimation();
    };
}