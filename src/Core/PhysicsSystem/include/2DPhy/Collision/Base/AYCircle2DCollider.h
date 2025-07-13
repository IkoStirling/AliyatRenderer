#pragma once
#include "BasePhy/IAYCollider.h"

class AYCircle2DCollider : public IAYCollider {
public:
    explicit AYCircle2DCollider(float radius = 0.5f);

    // ��״����
    ShapeType getShapeType() const override { return ShapeType::Circle2D; }

    // ���з���
    float getRadius() const { return _radius; }
    void setRadius(float radius) { _radius = radius; }

private:
    float _radius;
};