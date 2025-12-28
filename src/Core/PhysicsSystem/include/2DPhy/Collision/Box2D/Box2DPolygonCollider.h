#pragma once
#include "2DPhy/Collision/Base/AYPolygon2DCollider.h"
#include "Box2DColliderBase.h"
namespace ayt::engine::physics
{
    class Box2DPolygonCollider : public Polygon2DCollider, public Box2DColliderBase
    {
    public:
        explicit Box2DPolygonCollider(const std::vector<math::Vector2>& vertices = {}) :
            Polygon2DCollider(vertices) {
        }

        void setVertices(const std::vector<math::Vector2>& vertices) override
        {
            Polygon2DCollider::setVertices(vertices);
            if (isValid()) updateShape(_shapeId);
        }

        void addVertex(const math::Vector2& vertex) override
        {
            Polygon2DCollider::addVertex(vertex);
            if (isValid()) updateShape(_shapeId);
        }

        void removeVertex(size_t index) override
        {
            if (index < _vertices.size()) {
                _vertices.erase(_vertices.begin() + index);
                _updateConvexity();
                if (isValid()) updateShape(_shapeId);
            }
        }

        void clearVertices() override
        {
            Polygon2DCollider::clearVertices();
            if (isValid()) updateShape(_shapeId);
        }

        void setOffset(const math::Vector2& offset) override
        {
            Polygon2DCollider::setOffset(offset);
            if (isValid()) updateShape(_shapeId);
        }

        b2ShapeId createShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override
        {
            if (_vertices.empty()) return b2_nullShapeId;

            b2Polygon polygon = createBox2DPolygon();
            return b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        void updateShape(b2ShapeId shapeId) const override
        {
            if (B2_IS_NULL(shapeId) || _vertices.empty()) return;

            b2Polygon polygon = createBox2DPolygon();
            b2Shape_SetPolygon(shapeId, &polygon);
        }

    private:
        b2Polygon createBox2DPolygon() const
        {
            if (_vertices.empty()) {
                // 返回一个默认的小多边形
                return b2MakeBox(0.1f, 0.1f);
            }

            // 计算凸包
            b2Hull hull = computeConvexHull();

            if (hull.count >= 3) {
                // 使用凸包创建多边形
                return b2MakePolygon(&hull, 0.0f);
            }
            else {
                // 如果凸包无效，使用AABB
                b2AABB aabb = computeAABB();
                float halfWidth = (aabb.upperBound.x - aabb.lowerBound.x) * 0.5f;
                float halfHeight = (aabb.upperBound.y - aabb.lowerBound.y) * 0.5f;
                b2Vec2 center = {
                    (aabb.upperBound.x + aabb.lowerBound.x) * 0.5f + _offset.x,
                    (aabb.upperBound.y + aabb.lowerBound.y) * 0.5f + _offset.y
                };
                return b2MakeOffsetBox(halfWidth, halfHeight, center, b2Rot{ 0.0f });
            }
        }

        b2Hull computeConvexHull() const
        {
            b2Hull hull;
            std::vector<b2Vec2> points(_vertices.size());

            // 应用偏移并转换为b2Vec2
            for (size_t i = 0; i < _vertices.size(); ++i) {
                points[i] = {
                    _vertices[i].x + _offset.x,
                    _vertices[i].y + _offset.y
                };
            }

            // 计算凸包
            hull = b2ComputeHull(points.data(), (int32_t)points.size());
            return hull;
        }

        b2AABB computeAABB() const
        {
            b2AABB aabb;
            aabb.lowerBound = { FLT_MAX, FLT_MAX };
            aabb.upperBound = { -FLT_MAX, -FLT_MAX };

            for (const auto& vertex : _vertices) {
                b2Vec2 point = { vertex.x + _offset.x, vertex.y + _offset.y };
                aabb.lowerBound = b2Min(aabb.lowerBound, point);
                aabb.upperBound = b2Max(aabb.upperBound, point);
            }

            return aabb;
        }
    };
}