#pragma once
#include "BasePhy/IAYCollider.h"
namespace ayt::engine::physics
{
    class Circle2DCollider : public ICollider {
    public:
        explicit Circle2DCollider(float radius = 0.5f);

        // 形状类型
        ShapeType getShapeType() const override { return ShapeType::Circle2D; }

        // 特有方法
        virtual float getRadius() const { return _radius; }
        virtual void setRadius(float radius) { _radius = radius; }

    protected:
        float _radius;
    };
}