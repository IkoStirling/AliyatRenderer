#pragma once
#include "ECPhysicsDependence.h"
#include "IAYPhysicsBody.h"

class IAYPhysicsWorld
{
public:
    virtual ~IAYPhysicsWorld() = default;

    virtual void step(float delta_time) = 0;

    virtual void setGravity(const glm::vec2& gravity) = 0;

    virtual IAYPhysicsBody* createBody(const glm::vec2& position,
        float rotation,
        IAYPhysicsBody::BodyType type) = 0;
    
};