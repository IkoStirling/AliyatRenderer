#pragma once
#include "BasePhy/IAYCollider.h"

class AYPolygon2DCollider : public IAYCollider {
public:
    explicit AYPolygon2DCollider(const std::vector<glm::vec2>& vertices = {});

    // 形状类型
    ShapeType getShapeType() const override { return ShapeType::Polygon2D; }

    // 特有方法
    const std::vector<glm::vec2>& getVertices() const { return _vertices; }
    void setVertices(const std::vector<glm::vec2>& vertices);

    bool isConvex() const { return _isConvex; }
    void setConvex(bool convex) { _isConvex = convex; }

    // 实用方法
    void addVertex(const glm::vec2& vertex);
    void removeVertex(size_t index);
    void clearVertices();

private:
    std::vector<glm::vec2> _vertices;
    bool _isConvex = true; // 默认为凸多边形

    void _updateConvexity(); // 自动计算凸性
};