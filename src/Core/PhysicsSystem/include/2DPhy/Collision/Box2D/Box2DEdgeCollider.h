#pragma once
#include "2DPhy/Collision/Base/AYEdge2DCollider.h"
#include "Box2DColliderBase.h"

class Box2DEdgeCollider : public AYEdge2DCollider, public Box2DColliderBase
{
public:
    explicit Box2DEdgeCollider(const glm::vec2& size = { 1.0f, 1.0f }) :
        AYEdge2DCollider(size) {}

    void setOffset(const glm::vec2& offset) override {
        AYEdge2DCollider::setOffset(offset);
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    void setVertices(const glm::vec2& v1, const glm::vec2& v2) override
    {
        AYEdge2DCollider::setVertices(v1, v2);
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    void setOneSided(bool oneSided) override
    {
        AYEdge2DCollider::setOneSided(oneSided);
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    void setAdjacentVertices(const glm::vec2& prevVertex, const glm::vec2& nextVertex) override
    {
        AYEdge2DCollider::setAdjacentVertices(prevVertex, nextVertex);
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    b2Shape* createBox2DShape()
    {
        b2EdgeShape* shape = new b2EdgeShape();
        updateBox2DShape(shape);
        return shape;
    }

    void updateBox2DShape(b2Shape* shape)
    {
        b2EdgeShape* edgeShape = dynamic_cast<b2EdgeShape*>(shape);
        if (!edgeShape) return;

        b2Vec2 v1(_vertex1.x + _offset.x, _vertex1.y + _offset.y);
        b2Vec2 v2(_vertex2.x + _offset.x, _vertex2.y + _offset.y);

        if (_isOneSided) {
            // 计算默认的相邻顶点（如果未明确设置）
            b2Vec2 dir = v2 - v1;
            b2Vec2 normal(-dir.y, dir.x);
            normal.Normalize();

            b2Vec2 v0 = _prevVertexSet ?
                b2Vec2(_prevVertex.x + _offset.x, _prevVertex.y + _offset.y) :
                v1 - normal;

            b2Vec2 v3 = _nextVertexSet ?
                b2Vec2(_nextVertex.x + _offset.x, _nextVertex.y + _offset.y) :
                v2 + normal;

            edgeShape->SetOneSided(v0, v1, v2, v3);
        }
        else {
            edgeShape->SetTwoSided(v1, v2);
        }
    }
};