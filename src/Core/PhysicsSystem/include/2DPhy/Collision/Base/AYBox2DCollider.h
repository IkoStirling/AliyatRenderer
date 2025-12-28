#pragma once
#include "BasePhy/IAYCollider.h"
namespace ayt::engine::physics
{
    class AYBox2DCollider : public IAYCollider
    {
    public:
        explicit AYBox2DCollider(const math::Vector2& size = { 1.0f, 1.0f });

        // 形状类型
        ShapeType getShapeType() const override { return ShapeType::Box2D; }

        // 特有方法
        virtual const math::Vector2& getSize() const { return _size; }
        virtual void setSize(const math::Vector2& size) { _size = size; }


    protected:
        math::Vector2 _size;
    };
}