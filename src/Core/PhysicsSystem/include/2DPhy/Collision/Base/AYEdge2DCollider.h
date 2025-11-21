#pragma once
#include "BasePhy/IAYCollider.h"

class AYEdge2DCollider : public IAYCollider {
public:
    explicit AYEdge2DCollider(const AYMath::Vector2& vertex1 = { 0,0 },
        const AYMath::Vector2& vertex2 = { 1,0 });

    // 形状类型
    ShapeType getShapeType() const override { return ShapeType::Edge2D; }

    // 位置偏移
    void setOffset(const AYMath::Vector2& offset) override;

    // 特有方法
    const AYMath::Vector2& getVertex1() const { return _vertex1; }
    const AYMath::Vector2& getVertex2() const { return _vertex2; }
    virtual void setVertices(const AYMath::Vector2& v1, const AYMath::Vector2& v2) { _vertex1 = v1; _vertex2 = v2; }

    bool isOneSided() const { return _isOneSided; }
    virtual void setOneSided(bool oneSided) { _isOneSided = oneSided; }

    virtual void setAdjacentVertices(const AYMath::Vector2& prevVertex, const AYMath::Vector2& nextVertex) {
        _prevVertex = prevVertex;
        _nextVertex = nextVertex;
        _isOneSided = true;
    }

    // 实用方法
    float getLength() const;
    AYMath::Vector2 getDirection() const;

protected:
    AYMath::Vector2 _vertex1;
    AYMath::Vector2 _vertex2;
    bool _isOneSided = false;

    AYMath::Vector2 _prevVertex;
    AYMath::Vector2 _nextVertex;
    bool _prevVertexSet = false;
    bool _nextVertexSet = false;
};