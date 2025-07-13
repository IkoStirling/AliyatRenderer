#pragma once
#include "BasePhy/IAYCollider.h"

class AYPolygon2DCollider : public IAYCollider {
public:
    explicit AYPolygon2DCollider(const std::vector<glm::vec2>& vertices = {});

    // ��״����
    ShapeType getShapeType() const override { return ShapeType::Polygon2D; }

    // ���з���
    const std::vector<glm::vec2>& getVertices() const { return _vertices; }
    void setVertices(const std::vector<glm::vec2>& vertices);

    bool isConvex() const { return _isConvex; }
    void setConvex(bool convex) { _isConvex = convex; }

    // ʵ�÷���
    void addVertex(const glm::vec2& vertex);
    void removeVertex(size_t index);
    void clearVertices();

private:
    std::vector<glm::vec2> _vertices;
    bool _isConvex = true; // Ĭ��Ϊ͹�����

    void _updateConvexity(); // �Զ�����͹��
};