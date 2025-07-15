#pragma once
#include <box2d/box2d.h>

class Box2DColliderBase 
{
public:
    virtual ~Box2DColliderBase() = default;

    // Box2D��״�����͸���
    virtual b2Shape* createBox2DShape() const = 0;
    virtual void updateBox2DShape(b2Shape* shape) const = 0;

    // ��Box2D fixture�Ĺ���
    void setFixture(b2Fixture* fixture) { _fixture = fixture; }
    b2Fixture* getFixture() const { return _fixture; }

protected:
    b2Fixture* _fixture = nullptr;
};