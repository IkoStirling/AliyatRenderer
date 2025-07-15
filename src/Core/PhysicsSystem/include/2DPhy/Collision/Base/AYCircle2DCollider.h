#pragma once
#include "BasePhy/IAYCollider.h"

class AYCircle2DCollider : public IAYCollider {
public:
    explicit AYCircle2DCollider(float radius = 0.5f);

    // ��״����
    ShapeType getShapeType() const override { return ShapeType::Circle2D; }

    // ���з���
    virtual float getRadius() const { return _radius; }
    virtual void setRadius(float radius) { _radius = radius; }

protected:
    float _radius;
};