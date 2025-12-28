#pragma once
#include "AYMathType.h"
#include "STTransform.h"
namespace ayt::engine::physics
{
    class IPhysical
    {
    public: // transform
        virtual const math::Transform& getTransform() const = 0;
        virtual void setTransform(const math::Transform& transform) = 0;

        virtual math::Vector3 getPosition() const = 0;
        virtual void setPosition(const math::Vector3& position) = 0;
        virtual void setPosition(const math::Vector2& position) = 0;

        virtual math::Vector3 getRotation() const = 0;
        virtual void setRotation(const math::Vector3& rotation) = 0;
        //void setRotation(float rotation) = 0;

        virtual math::Vector3 getScale() const = 0;
        virtual void setScale(const math::Vector3& scale) = 0;
        virtual void setScale(const math::Vector2& scale) = 0;

    protected:
        math::Transform _transform;
    };
}