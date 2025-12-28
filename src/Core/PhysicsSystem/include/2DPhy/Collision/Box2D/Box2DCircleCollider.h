#pragma once
#include "2DPhy/Collision/Base/AYCircle2DCollider.h"
#include "Box2DColliderBase.h"
namespace ayt::engine::physics
{
    class Box2DCircleCollider : public Circle2DCollider, public Box2DColliderBase
    {
    public:
        explicit Box2DCircleCollider(float radius = 0.5f) :
            Circle2DCollider(radius) {
        }

        void setRadius(float radius) override
        {
            _radius = radius;
            if (isValid()) updateShape(_shapeId);
        }

        void setOffset(const math::Vector2& offset) override
        {
            _offset = offset;
            if (isValid()) updateShape(_shapeId);
        }

        b2ShapeId createShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override
        {
            b2Circle circle = createBox2DCircle();
            return b2CreateCircleShape(bodyId, &shapeDef, &circle);
        }

        void updateShape(b2ShapeId shapeId) const override
        {
            if (B2_IS_NULL(shapeId)) return;

            b2Circle circle = createBox2DCircle();
            b2Shape_SetCircle(shapeId, &circle);
        }

    private:
        b2Circle createBox2DCircle() const
        {
            b2Circle circle;
            circle.center = { _offset.x, _offset.y };
            circle.radius = _radius;
            return circle;
        }

    };
}