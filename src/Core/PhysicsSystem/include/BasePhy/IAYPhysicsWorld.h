#pragma once
#include "ECPhysicsDependence.h"
#include "IAYPhysicsBody.h"

class IAYPhysicsWorld
{
public:
    virtual ~IAYPhysicsWorld() = default;

    virtual void step(float delta_time, int velocity_iterations = 8, int position_iterations = 3) = 0;

    virtual void setGravity(const glm::vec2& gravity) = 0;

    virtual void raycast(const glm::vec2& start, const glm::vec2& end,
        std::function<bool(IAYPhysicsBody*, const glm::vec2&)> callback) = 0;

    virtual IAYPhysicsBody* createBody(EntityID entity,
        const glm::vec2& position = glm::vec2(0),
        float rotation = 0,
        IAYPhysicsBody::BodyType type = IAYPhysicsBody::BodyType::Dynamic) = 0;
    
    virtual void setTransform(EntityID entity, const STTransform& transform) = 0;
    virtual const STTransform& getTransform(EntityID entity) = 0;
};