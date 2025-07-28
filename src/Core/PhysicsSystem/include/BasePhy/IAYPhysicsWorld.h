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

    virtual IAYPhysicsBody* createBody(IAYPhysical* game_object,
        const glm::vec2& position,
        float rotation,
        IAYPhysicsBody::BodyType type) = 0;
    
    virtual void syncPhysicsToLogic() = 0;
};