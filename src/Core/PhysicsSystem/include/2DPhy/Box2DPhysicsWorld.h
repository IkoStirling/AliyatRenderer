#pragma once
#include "BasePhy/IAYPhysicsWorld.h"
#include "Box2DPhysicsBody.h"

class Box2DPhysicsWorld : public IAYPhysicsWorld
{
public:
    Box2DPhysicsWorld() : _world(b2Vec2(0, -9.8f)) {}

    void step(float deltaTime) override {
        _world.Step(deltaTime, 8, 3);
    }

    void setGravity(const glm::vec2& gravity) override {
        _world.SetGravity(b2Vec2(gravity.x, gravity.y));
    }

    IAYPhysicsBody* createBody(const glm::vec2& position, float rotation, IAYPhysicsBody::BodyType type) override {
        return new Box2DPhysicsBody(_world, position, rotation, type);
    }

private:
    b2World _world;
};