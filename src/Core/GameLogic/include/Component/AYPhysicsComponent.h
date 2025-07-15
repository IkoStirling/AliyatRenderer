#pragma once
#include "IAYComponent.h"
#include "AYPhysicsSystem.h"

class AYPhysicsComponent : public IAYComponent, public AYPhysicsSystem
{
public:

    virtual void beginPlay() override
    {
        _physicsBody = GET_CAST_MODULE(AYPhysicsSystem, "PhysicsSystem")->
            getPhysicsWorld(AYPhysicsSystem::WorldType::AY2D)->createBody(glm::vec2(), 0.f, IAYPhysicsBody::BodyType::Dynamic);
    }

    virtual void update(float delta_time) override
    {

    }

    virtual void endPlay() override
    {

    }

    void setBodyType(IAYPhysicsBody::BodyType type)
    {
        if (_physicsBody)
            _physicsBody->setType(type);
    }

    void addCollider(std::shared_ptr<IAYCollider> collider)
    {
        _colliders.push_back(collider);
    }

    void removeCollider(std::shared_ptr<IAYCollider> collider)
    {
        _colliders.erase(collider);
    }

    void applyForce(const glm::vec2& force)
    {
        _physicsBody->applyForce(force);
    }


private:
    std::unique_ptr<IAYPhysicsBody> _physicsBody;
    std::vector<std::shared_ptr<IAYCollider>> _colliders;
};