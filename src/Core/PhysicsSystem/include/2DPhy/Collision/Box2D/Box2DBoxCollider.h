#pragma once
#include "2DPhy/Collision/Base/AYBox2DCollider.h"
#include "Box2DColliderBase.h"

class Box2DBoxCollider : public AYBox2DCollider, public Box2DColliderBase
{
public:
    explicit Box2DBoxCollider(const AYMath::Vector2& size = { 1.0f, 1.0f }) :
        AYBox2DCollider(size) {}

    // 特有方法
    void setSize(const AYMath::Vector2& size) override
    {
        _size = size;
        if (isValid()) {
            updateShape(_shapeId);
        }
    }

    void setRotation(const AYMath::Vector3& rotation) override
    {
        _rotation = rotation;
        if (isValid()) {
            updateShape(_shapeId);
        }
    }

    b2ShapeId createShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override
    {
        // 创建多边形形状
        b2Polygon polygon = createBoxPolygon();
        _shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        return _shapeId;
    }

    void updateShape(b2ShapeId shapeId) const override
    {
        if (B2_IS_NULL(shapeId)) return;

        // 更新多边形形状
        b2Polygon polygon = createBoxPolygon();
        b2Shape_SetPolygon(shapeId, &polygon);
    }

private:
    // 创建Box2D多边形
    b2Polygon createBoxPolygon() const
    {
        float halfWidth = _size.x * 0.5f;
        float halfHeight = _size.y * 0.5f;

        // 如果有偏移或旋转，使用偏移版本的函数
        if (_offset != AYMath::Vector2(0.0f, 0.0f) || _rotation != AYMath::Vector3(0.0f)) {
            b2Vec2 center = { _offset.x, _offset.y };
            b2Rot rotation = AYMath::Adapter::toBox2DRot(_rotation.z);
            // 如果有圆角半径（如果需要的话）
            // float radius = 0.1f; // 示例圆角半径
            // return b2MakeOffsetRoundedBox(halfWidth, halfHeight, center, b2Rot{0.0f}, radius);

            // 使用偏移盒子
            return b2MakeOffsetBox(halfWidth, halfHeight, center, rotation);
        }
        else {
            // 使用普通盒子
            return b2MakeBox(halfWidth, halfHeight);
        }
    }
};