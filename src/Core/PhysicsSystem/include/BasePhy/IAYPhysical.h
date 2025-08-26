#pragma once
#include "STTransform.h"

class IAYPhysical
{
public: // transform
    virtual const STTransform& getTransform() const = 0;
    virtual void setTransform(const STTransform& transform) = 0;

    virtual glm::vec3 getPosition() const = 0;
    virtual void setPosition(const glm::vec3& position) = 0;
    virtual void setPosition(const glm::vec2& position) = 0;

    virtual glm::vec3 getRotation() const = 0;
    virtual void setRotation(const glm::vec3& rotation) = 0;
    //void setRotation(float rotation) = 0;

    virtual glm::vec3 getScale() const = 0;
    virtual void setScale(const glm::vec3& scale) = 0;
    virtual void setScale(const glm::vec2& scale) = 0;

protected:
    STTransform _transform;
};