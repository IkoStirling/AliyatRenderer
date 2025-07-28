#pragma once
#include "IAYComponent.h"
#include "AYPhysicsSystem.h"


class AYPhysicsComponent : public IAYComponent
{
public:

    virtual void beginPlay() override
    {
        auto physicsSystem = GET_CAST_MODULE(AYPhysicsSystem, "PhysicsSystem");

        _physicsBody = physicsSystem->getPhysicsWorld(AYPhysicsSystem::WorldType::AY2D)
            ->createBody(
                getOwner(),
                glm::vec2(),
                0.f,
                _bodyType);

        for (auto& collider : _colliders) {
            _physicsBody->addCollider(collider.get());
        }
    }

    virtual void update(float delta_time) override {}

    virtual void endPlay() override {}

    // 基础物理控制
    IAYPhysicsBody* getPhysicsBody() { return _physicsBody; }

    // 状态获取
    bool isGrounded() const {
        return _groundContactCount == 0;
    }

    void setBodyType(IAYPhysicsBody::BodyType type)
    {
        _bodyType = type;
        if (_physicsBody)
            _physicsBody->setType(_bodyType);
    }

    void addCollider(std::shared_ptr<IAYCollider> collider)
    {
        _colliders.push_back(collider);
    }

    void removeCollider(std::shared_ptr<IAYCollider> collider)
    {
        auto it = std::find(_colliders.begin(), _colliders.end(), collider);

        if (it != _colliders.end())
        {
            _colliders.erase(it);
        }
    }


protected:
    IAYPhysicsBody* _physicsBody = nullptr;
    IAYPhysicsBody::BodyType _bodyType;
    std::vector<std::shared_ptr<IAYCollider>> _colliders;
    float _moveSpeed = 5.0f;
    float _jumpForce = 10.0f;
    bool _isGrounded = false;
    int _groundContactCount = 0;

    
};