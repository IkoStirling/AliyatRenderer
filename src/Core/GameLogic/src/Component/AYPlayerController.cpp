#include "Component/AYPlayerController.h"
#include "Component/AYPhysicsComponent.h"
#include "AYInputSystem.h"
#include "AYGameObject.h"

namespace ayt::engine::game
{
    using namespace ::ayt::engine::physics;
    using namespace ::ayt::engine::input;

    void PlayerController::beginPlay() {
        _physics = getOwner()->getComponent<PhysicsComponent>();
        if (!_physics) {
            _physics = getOwner()->addComponent<PhysicsComponent>("_physics");
        }
    }

    void PlayerController::update(float delta_time) {
        if (!_physics) return;

        // 更新跳跃冷却时间
        if (_jumpCooldownTimer > 0.0f) {
            _jumpCooldownTimer -= delta_time;
            if (_jumpCooldownTimer < 0.0f) {
                _jumpCooldownTimer = 0.0f;
            }
        }

        // 输入处理
        math::Vector2 input = _getMovementInput();

        // 状态更新
        _updateState();
        _updateMovement(input, delta_time);
        _updateJump();
        _updateAnimation();
    }

    void PlayerController::endPlay(){}

    void PlayerController::jump() { _shouldJump = true; }

    void PlayerController::setMoveSpeed(float speed) { _moveSpeed = speed; }
    void PlayerController::setRunSpeed(float speed) { _runSpeed = speed; }
    void PlayerController::setJumpForce(float force) { _jumpForce = force; }

    void PlayerController::_updateState()
    {
        _lastState = _currentState;
        auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");
        auto physicsSystem = GET_CAST_MODULE(PhysicsSystem, "PhysicsSystem");
        auto* body = _physics->getPhysicsBody();
        if (auto* bbody = dynamic_cast<Box2DPhysicsBody*>(body))
        {
            if (auto* world = dynamic_cast<Box2DPhysicsWorld*>(physicsSystem->getPhysicsWorld(WorldType::AY2D)))
            {
                auto info = world->checkGroundContact(body, 0.03f);

                // 获取速度
                math::Vector2 vel = body->getLinearVelocity();
                float velY = vel.y;
                float velX = vel.x;
                bool isMoving = glm::length(math::Vector2(velX, 0.0f)) > 0.1f; // 水平移动阈值
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

    math::Vector2 PlayerController::_getMovementInput() {
        auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");
        float x = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftX })
            + (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_D }) ? 1.f : 0.f)
            - (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_A }) ? 1.f : 0.f);
        float y = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftY })
            - (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_S }) ? 1.f : 0.f)
            + (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_W }) ? 1.f : 0.f);
        auto result = math::Vector2(x, y);
        if (glm::length(result) > 0)
            return glm::normalize(math::Vector2(x, y));
        else
            return result;

    }

    void PlayerController::_updateMovement(const math::Vector2& input, float deltaTime) {
        float targetSpeedX = input.x * (_currentState == PlayerState::Running ? _runSpeed : _moveSpeed);

        // 3. 平滑过渡当前速度
        float currentVelX = _physics->getPhysicsBody()->getLinearVelocity().x;
        float newVelX = (currentVelX + targetSpeedX) / 2;

        // 4. 保持垂直速度不变（重力作用）
        _physics->getPhysicsBody()->setLinearVelocity(math::Vector2(
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
            AYLOG_INFO("[PlayerController] \tInput:({:.4f},{:.4f})\tVelocity:({:.4f},{:.4f})\tPosition({:.4f},{:.4f})",
                input.x, input.y,
                newVelX,
                _physics->getPhysicsBody()->getLinearVelocity().y,
                pos.x, pos.y);
        }


    }

    bool PlayerController::_shouldJumpNow()
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

    void PlayerController::_updateJump()
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
            _physics->getPhysicsBody()->setAngularVelocity(0);
            _physics->getPhysicsBody()->applyImpulse(math::Vector2(0, _jumpForce));
            _currentJumpCount++;
            _jumpCooldownTimer = _jumpCooldownTime;
        }
    }

    void PlayerController::_updateAnimation()
    {
    }
}