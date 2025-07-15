#pragma once
#include "2DPhy/Collision/Base/AYPolygon2DCollider.h"
#include "Box2DColliderBase.h"

class Box2DPolygonCollider : public AYPolygon2DCollider, public Box2DColliderBase
{
public:
    explicit Box2DPolygonCollider(const std::vector<glm::vec2>& vertices = {}) :
        AYPolygon2DCollider(vertices) {}

    void setVertices(const std::vector<glm::vec2>& vertices) override
    {
        AYPolygon2DCollider::setVertices(vertices);
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    void addVertex(const glm::vec2& vertex) override
    {
        AYPolygon2DCollider::addVertex(vertex);
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    void removeVertex(size_t index) override
    {
        if (index < _vertices.size()) {
            _vertices.erase(_vertices.begin() + index);
            _updateConvexity();
            if (_fixture) updateBox2DShape(_fixture->GetShape());
        }
    }

    void clearVertices() override
    {
        AYPolygon2DCollider::clearVertices();
        if (_fixture) updateBox2DShape(_fixture->GetShape());
    }

    b2Shape* createBox2DShape() const override {
        b2PolygonShape* shape = new b2PolygonShape();
        updateBox2DShape(shape);
        return shape;
    }

    void updateBox2DShape(b2Shape* shape) const override {
        b2PolygonShape* polygonShape = dynamic_cast<b2PolygonShape*>(shape);
        if (polygonShape && !_vertices.empty()) {
            std::vector<b2Vec2> b2Vertices(_vertices.size());
            for (size_t i = 0; i < _vertices.size(); ++i) {
                b2Vertices[i].Set(_vertices[i].x + _offset.x,
                    _vertices[i].y + _offset.y);
            }
            polygonShape->Set(b2Vertices.data(), static_cast<int32>(_vertices.size()));
        }
    }
};