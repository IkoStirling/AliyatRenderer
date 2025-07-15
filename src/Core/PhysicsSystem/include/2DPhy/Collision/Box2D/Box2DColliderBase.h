#pragma once
#include <box2d/box2d.h>

class Box2DColliderBase 
{
public:
    virtual ~Box2DColliderBase() = default;

    // Box2D形状创建和更新
    virtual b2Shape* createBox2DShape() const = 0;
    virtual void updateBox2DShape(b2Shape* shape) const = 0;

    // 与Box2D fixture的关联
    void setFixture(b2Fixture* fixture) { _fixture = fixture; }
    b2Fixture* getFixture() const { return _fixture; }

protected:
    b2Fixture* _fixture = nullptr;
};