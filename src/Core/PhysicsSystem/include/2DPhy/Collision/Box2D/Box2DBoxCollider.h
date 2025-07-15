#pragma once
#include "2DPhy/Collision/Base/AYBox2DCollider.h"
#include "Box2DColliderBase.h"

class Box2DBoxCollider : public AYBox2DCollider, public Box2DColliderBase
{
public:
    explicit Box2DBoxCollider(const glm::vec2& size = { 1.0f, 1.0f }) :
        AYBox2DCollider(size) {}

    // 特有方法
    void setSize(const glm::vec2& size) override
    { 
        _size = size;
        if (_fixture)
            updateBox2DShape(_fixture->GetShape());
    }

    b2Shape* createBox2DShape() const override
    {
        b2PolygonShape* shape = new b2PolygonShape();
        updateBox2DShape(shape);
        return shape;
    }

    void updateBox2DShape(b2Shape* shape) const override
    {
        b2PolygonShape* boxShape = dynamic_cast<b2PolygonShape*>(shape);
        if (boxShape) {
            b2Vec2 boxSize(_size.x * 0.5f, _size.y * 0.5f);
            boxShape->SetAsBox(boxSize.x, boxSize.y,
                b2Vec2(_offset.x, _offset.y), 0.0f);
        }
    }
};