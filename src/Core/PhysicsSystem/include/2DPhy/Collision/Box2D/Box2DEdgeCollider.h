#pragma once
#include "2DPhy/Collision/Base/AYEdge2DCollider.h"
#include "Box2DColliderBase.h"
namespace ayt::engine::physics
{
    class Box2DEdgeCollider : public AYEdge2DCollider, public Box2DColliderBase
    {
    public:
        explicit Box2DEdgeCollider(const math::Vector2& size = { 1.0f, 1.0f }) :
            AYEdge2DCollider(size) {
        }

        void setOffset(const math::Vector2& offset) override {
            AYEdge2DCollider::setOffset(offset);
            if (isValid()) updateShape(_shapeId);
        }

        void setVertices(const math::Vector2& v1, const math::Vector2& v2) override
        {
            AYEdge2DCollider::setVertices(v1, v2);
            if (isValid()) updateShape(_shapeId);
        }

        void setOneSided(bool oneSided) override
        {
            AYEdge2DCollider::setOneSided(oneSided);
            if (isValid()) updateShape(_shapeId);
        }

        void setAdjacentVertices(const math::Vector2& prevVertex, const math::Vector2& nextVertex) override
        {
            AYEdge2DCollider::setAdjacentVertices(prevVertex, nextVertex);
            if (isValid()) updateShape(_shapeId);
        }

        b2ShapeId createShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override
        {
            b2Segment segment = createBox2DSegment();
            return b2CreateSegmentShape(bodyId, &shapeDef, &segment);
        }

        void updateShape(b2ShapeId shapeId) const override
        {
            if (B2_IS_NULL(shapeId)) return;

            b2Segment segment = createBox2DSegment();
            b2Shape_SetSegment(shapeId, &segment);
        }

    private:
        b2Segment createBox2DSegment() const
        {
            b2Segment segment;

            // 应用偏移
            segment.point1 = { _vertex1.x + _offset.x, _vertex1.y + _offset.y };
            segment.point2 = { _vertex2.x + _offset.x, _vertex2.y + _offset.y };

            if (_isOneSided) {
                // 计算默认的相邻顶点（如果未明确设置）
                b2Vec2 dir = segment.point2 - segment.point1;
                b2Vec2 normal = { -dir.y, dir.x };
                normal = b2Normalize(normal);

                segment.point1 = _prevVertexSet ?
                    b2Vec2{ _prevVertex.x + _offset.x, _prevVertex.y + _offset.y } :
                    segment.point1 - normal;

                segment.point2 = _nextVertexSet ?
                    b2Vec2{ _nextVertex.x + _offset.x, _nextVertex.y + _offset.y } :
                    segment.point2 + normal;
            }
            else {
                // 双边边缘，清除幽灵点
                segment.point1 = segment.point1;
                segment.point2 = segment.point2;
            }

            return segment;
        }
    };
}