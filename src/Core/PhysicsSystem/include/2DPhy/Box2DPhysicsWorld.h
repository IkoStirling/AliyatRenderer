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

    IAYPhysicsBody* createBody(EntityID entity,
        const glm::vec2& position,
        float rotation,
        IAYPhysicsBody::BodyType type) override 
    {
        std::lock_guard<std::mutex> lock(_bbodyMutex);
        auto [iter, inserted] = _bodies.try_emplace(entity, std::make_unique<Box2DPhysicsBody>(_world, position, rotation, type));
        auto& body = iter->second;
        body->setOwningEntity(entity);
        body->setType(type);
        return body.get();
    }

    void setTransform(EntityID entity, const STTransform& transform) override
    {
        auto& body = _bodies[entity];
        body->setTransform(transform);
    }
    const STTransform& getTransform(EntityID entity) override
    {
        auto& body = _bodies[entity];
        return body->getTransform();
    }

private:
    b2World _world;
    std::unordered_map<EntityID, std::unique_ptr<Box2DPhysicsBody>> _bodies;
    std::mutex _bbodyMutex;
};