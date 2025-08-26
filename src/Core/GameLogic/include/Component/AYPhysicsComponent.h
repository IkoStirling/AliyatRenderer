#pragma once
#include "IAYComponent.h"
#include "AYPhysicsSystem.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"

class AYPhysicsComponent : public IAYComponent
{
public:
    virtual void beginPlay() override;

    virtual void update(float delta_time) override; 

    virtual void endPlay() override {}

    // 基础物理控制
    IAYPhysicsBody* getPhysicsBody() { return _physicsBody; }

    void setBodyType(IAYPhysicsBody::BodyType type);


    void addCollider(std::shared_ptr<IAYCollider> collider);


    void removeCollider(std::shared_ptr<IAYCollider> collider);

    void setPhysicsMode(bool is3D) { _is3D = is3D; }

protected:
    IAYPhysicsBody* _physicsBody = nullptr;
    IAYPhysicsBody::BodyType _bodyType;
    std::vector<std::shared_ptr<IAYCollider>> _colliders;
    bool _is3D = false;

    
};