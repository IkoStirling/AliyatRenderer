#pragma once
#include "2DPhy/Collision/Base/AYCircle2DCollider.h"
#include "Box2DColliderBase.h"

class Box2DCircleCollider : public AYCircle2DCollider, public Box2DColliderBase
{
public:
    explicit Box2DCircleCollider(float radius = 0.5f) :
        AYCircle2DCollider(radius) {}


    void setRadius(float radius) override
    {
        _radius = radius;
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    b2Shape* createBox2DShape() const override
    {
        b2CircleShape* shape = new b2CircleShape();
        updateBox2DShape(shape);
        return shape;
    }

    void updateBox2DShape(b2Shape* shape) const override
    {
        b2CircleShape* circleShape = dynamic_cast<b2CircleShape*>(shape);
        if (circleShape) {
            circleShape->m_radius = _radius;
            circleShape->m_p.Set(_offset.x, _offset.y);
        }
    }
};