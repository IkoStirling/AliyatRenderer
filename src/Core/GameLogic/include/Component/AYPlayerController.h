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

    void update(float deltaTime) override {
        if (!_physics) return;

        // 输入处理
        glm::vec2 input = _getMovementInput();

        // 状态更新
        _updateMovement(input, deltaTime);
        _updateJump();
        _updateAnimation();
    }

    virtual void endPlay() override {}

    void setMoveSpeed(float speed) { _moveSpeed = speed; }
    void setJumpForce(float force) { _jumpForce = force; }
protected:
    AYPhysicsComponent* _physics = nullptr;
    float _moveSpeed = 5.0f;
    float _jumpForce = 10.0f;

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
        // 转换为物理系统坐标
        glm::vec2 physicsInput = _engineToPhysics(input);

        float targetSpeedX = physicsInput.x * _moveSpeed * PIXELS_PER_METER;

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
        if (time > 1.f)
        {
            time -= 1.f;
            auto pos = _physics->getPhysicsBody()->getPosition();
            std::cout << std::fixed << std::setprecision(4)
                << "[AYPlayerController]  \tInput: (" << input.x << ", " << input.y
                << ")  \tVelocity: (" << newVelX
                << ", " << _physics->getPhysicsBody()->getLinearVelocity().y
                << ")  \tPosition(" << pos.x << ", " << pos.y << ")\n";
        }


    }

    bool _shouldJump() {
        auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");
        return _physics->getPhysicsBody()->getLinearVelocity().y <= 0.01f  &&
        (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_SPACE }) ||
            inputSystem->isActionActive("default.GamePad_A"));
        return _physics->isGrounded() &&
            (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_SPACE }) ||
                inputSystem->isActionActive("default.GamePad_A"));
    }

    void _updateJump() {
        if (_shouldJump())
        {
            _physics->getPhysicsBody()->applyImpulse(glm::vec2(0, _jumpForce));
        }
    }

    void _updateAnimation() 
    {
    }
    // 坐标转换函数
    glm::vec2 _engineToPhysics(const glm::vec2& engineVec) {
        return engineVec / PIXELS_PER_METER;
    }

    glm::vec2 _physicsToEngine(const glm::vec2& physicsVec) {
        return physicsVec * PIXELS_PER_METER;
    }
};