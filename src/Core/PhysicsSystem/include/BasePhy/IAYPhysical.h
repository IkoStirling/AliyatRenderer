#pragma once
#include "AYMathType.h"
#include "STTransform.h"

class IAYPhysical
{
public: // transform
    virtual const STTransform& getTransform() const = 0;
    virtual void setTransform(const STTransform& transform) = 0;

    virtual AYMath::Vector3 getPosition() const = 0;
    virtual void setPosition(const AYMath::Vector3& position) = 0;
    virtual void setPosition(const AYMath::Vector2& position) = 0;

    virtual AYMath::Vector3 getRotation() const = 0;
    virtual void setRotation(const AYMath::Vector3& rotation) = 0;
    //void setRotation(float rotation) = 0;

    virtual AYMath::Vector3 getScale() const = 0;
    virtual void setScale(const AYMath::Vector3& scale) = 0;
    virtual void setScale(const AYMath::Vector2& scale) = 0;

protected:
    STTransform _transform;
};