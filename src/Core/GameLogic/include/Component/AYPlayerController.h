#pragma once
#include "AYPhysicsComponent.h"

class AYPlayerController : public IAYComponent
{
public:
    void beginPlay() override {
        _physics = getOwner()->getComponent<AYPhysicsComponent>();
        if (!_physics) {
            _physics = getOwner()->addComponent<AYPhysicsComponent>("_physics");
        }
    }

    void update(float delta_time) override {
        if (!_physics) return;

        // 更新跳跃冷却时间
        if (_jumpCooldownTimer > 0.0f) {
            _jumpCooldownTimer -= delta_time;
            if (_jumpCooldownTimer < 0.0f) {
                _jumpCooldownTimer = 0.0f;
            }
        }

        // 输入处理
        glm::vec2 input = _getMovementInput();

        // 状态更新
        _updateState();
        _updateMovement(input, delta_time);
        _updateJump();
        _updateAnimation();
    }

    virtual void endPlay() override {}

    void jump() { _shouldJump = true; }

    void setMoveSpeed(float speed) { _moveSpeed = speed; }
    void setRunSpeed(float speed) { _runSpeed = speed; }
    void setJumpForce(float force) { _jumpForce = force; }
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

    void _updateState()
    {
        _lastState = _currentState;
        auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");
        auto physicsSystem = GET_CAST_MODULE(AYPhysicsSystem, "PhysicsSystem");
        auto* body = _physics->getPhysicsBody();
        if (auto* bbody = dynamic_cast<Box2DPhysicsBody*>(body))
        {
            if (auto* world = dynamic_cast<Box2DPhysicsWorld*>(physicsSystem->getPhysicsWorld(WorldType::AY2D)))
            {
                auto info = world->checkGroundContact(body, 0.03f);

                // 获取速度
                glm::vec2 vel = body->getLinearVelocity();
                float velY = vel.y;
                float velX = vel.x;
                bool isMoving = glm::length(glm::vec2(velX, 0.0f)) > 0.1f; // 水平移动阈值
                bool isRunning = inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_LEFT_SHIFT });

                _isGrounded = info.isGrounded;
                if (!_isGrounded)
                {
                    if (velY > 0.1f)
                        _currentState = PlayerState::Jumping;  // 向上，起跳中
                    else
                        _currentState = PlayerState::Falling;  // 向下，下落中
                }
                else
                {
                    if (_lastState == PlayerState::Falling || _lastState == PlayerState::Jumping)
                    {
                        _currentState = PlayerState::Landed;
                        _currentJumpCount = 0;
                    }
                    else if (isMoving)
                    {
                        if (isRunning)
                            _currentState = PlayerState::Running;
                        else
                            _currentState = PlayerState::Moving;
                    }
                    else
                        _currentState = PlayerState::Idle;
                }
            }
        }
    }

    glm::vec2 _getMovementInput() {
        auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");
        float x = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftX })
            + (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_D }) ? 1.f : 0.f)
            - (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_A }) ? 1.f : 0.f);
        float y = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftY })
            - (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_S }) ? 1.f : 0.f)
            + (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_W }) ? 1.f : 0.f);
        auto result = glm::vec2(x, y);
        if (glm::length(result) > 0)
            return glm::normalize(glm::vec2(x, y)); 
        else
            return result;
        
    }

    void _updateMovement(const glm::vec2& input, float deltaTime) {
        float targetSpeedX = input.x * (_currentState == PlayerState::Running ? _runSpeed : _moveSpeed);

        // 3. 平滑过渡当前速度
        float currentVelX = _physics->getPhysicsBody()->getLinearVelocity().x;
        float newVelX = (currentVelX+ targetSpeedX)/2;

        // 4. 保持垂直速度不变（重力作用）
        _physics->getPhysicsBody()->setLinearVelocity(glm::vec2(
            newVelX,
            _physics->getPhysicsBody()->getLinearVelocity().y
        ));

        // 5. 调试输出（临时）
        static float time = 0;
        time += deltaTime;
        if (time > 5.f)
        {
            time -= 5.f;
            auto pos = _physics->getPhysicsBody()->getPosition();
            std::cout << std::fixed << std::setprecision(4)
                << "[AYPlayerController]  \tInput: (" << input.x << ", " << input.y
                << ")  \tVelocity: (" << newVelX
                << ", " << _physics->getPhysicsBody()->getLinearVelocity().y
                << ")  \tPosition(" << pos.x << ", " << pos.y << ")\n";
        }


    }

    bool _shouldJumpNow() 
    {
        auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");

        // 无论有没有跳跃，状态都会重置，避免输入缓冲
        auto shouldJump = _shouldJump;
        _shouldJump = false;

        if (!_isGrounded && _maxJumpCount == 1)
        {
            return false;
        }

        if (_currentJumpCount >= _maxJumpCount)
        {
            return false;
        }

        return shouldJump;
    }

    void _updateJump() 
    {

        // 如果还在跳跃冷却中，不允许跳跃
        if (_jumpCooldownTimer > 0.0f) {
            return;
        }

        if (_shouldJumpNow())
        {
            auto vel = _physics->getPhysicsBody()->getLinearVelocity();
            vel.y = 0;
            _physics->getPhysicsBody()->setLinearVelocity(vel);
            _physics->getPhysicsBody()->applyImpulse(glm::vec2(0, _jumpForce));
            _currentJumpCount++;
            _jumpCooldownTimer = _jumpCooldownTime;
        }
    }

    void _updateAnimation() 
    {
    }
};