#pragma once
#include "BasePhy/IAYCollider.h"

class AYCircle2DCollider : public IAYCollider {
public:
    explicit AYCircle2DCollider(float radius = 0.5f);

    // 形状类型
    ShapeType getShapeType() const override { return ShapeType::Circle2D; }

    // 特有方法
    float getRadius() const { return _radius; }
    void setRadius(float radius) { _radius = radius; }

private:
    float _radius;
};