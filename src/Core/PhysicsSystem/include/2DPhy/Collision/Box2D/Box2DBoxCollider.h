#pragma once
#include "ECPhysicsDependence.h"

class IAYCollider
{
public:
    virtual void SetSize(const Vector2f& size) = 0; //   ”√”⁄Box/Circleµ»
    virtual void SetTrigger(bool isTrigger) = 0;
    virtual ~IAYCollider() = default;
};