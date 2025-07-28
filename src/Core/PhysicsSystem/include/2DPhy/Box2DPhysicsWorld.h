#pragma once
#include "BasePhy/IAYPhysicsWorld.h"
#include "Box2DPhysicsBody.h"

class Box2DPhysicsWorld : public IAYPhysicsWorld
{
public:
    Box2DPhysicsWorld() : _world(b2Vec2(0, -9.8f)) {}

    void step(float deltaTime, int velocity_iterations = 8, int position_iterations = 3) override {
        _world.Step(deltaTime, velocity_iterations, position_iterations);
    }

    void setGravity(const glm::vec2& gravity) override {
        _world.SetGravity(b2Vec2(gravity.x, gravity.y));
    }

    void raycast(const glm::vec2& start, const glm::vec2& end,
        std::function<bool(IAYPhysicsBody*, const glm::vec2&)> callback) override
    {

    }

    IAYPhysicsBody* createBody(IAYPhysical* game_object,
        const glm::vec2& position,
        float rotation,
        IAYPhysicsBody::BodyType type) override 
    {
        std::lock_guard<std::mutex> lock(_bbodyMutex);
        _bbodys.push_back(std::make_unique<Box2DPhysicsBody>(_world, position, rotation, type));
        auto body = _bbodys.back().get();
        body->setPhysicalObject(game_object);
        body->setType(type);
        return body;
    }

    void syncPhysicsToLogic() override
    {
        for (auto& body : _bbodys)
        {
            if (body->isDynamic())
            {
                glm::vec2 position = glmToEngine(body->getPosition());
                float rotation = body->getB2Body()->GetTransform().q.GetAngle();
                body->getPhysicalObject()->setPosition(position);
                body->getPhysicalObject()->setRotation(rotation);
                //std::cout << "position: \t(" << body->getPosition().x << ", " << body->getPosition().y << ")\n";
            }
        }
    }

private:
    b2World _world;
    std::vector<std::unique_ptr<Box2DPhysicsBody>> _bbodys;
    std::mutex _bbodyMutex;
};