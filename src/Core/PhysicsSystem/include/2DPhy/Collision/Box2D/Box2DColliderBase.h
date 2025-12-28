#pragma once
#include "Adapter/Box2DAdapter.h"
namespace ayt::engine::physics
{
    class Box2DColliderBase
    {
    public:
        virtual ~Box2DColliderBase() = default;

        // Box2D形状创建和更新 - 改为使用b2ShapeId
        virtual b2ShapeId createShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) = 0;
        virtual void updateShape(b2ShapeId shapeId) const = 0;

        // 与Box2D形状的关联
        void setShapeId(b2ShapeId shapeId) { _shapeId = shapeId; }
        b2ShapeId getShapeId() const { return _shapeId; }

        // 检查形状ID是否有效
        bool isValid() const { return B2_IS_NON_NULL(_shapeId); }

    protected:
        b2ShapeId _shapeId = b2_nullShapeId;
    };
}