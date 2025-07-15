#include "2DPhy/Collision/Base/AYPolygon2DCollider.h"

AYPolygon2DCollider::AYPolygon2DCollider(const std::vector<glm::vec2>& vertices)
    : _vertices(vertices) {
    _updateConvexity();
}

void AYPolygon2DCollider::setVertices(const std::vector<glm::vec2>& vertices) {
    _vertices = vertices;
    _updateConvexity();
}

void AYPolygon2DCollider::_updateConvexity() {
    if (_vertices.size() < 3) {
        _isConvex = false;  // ����3�����㲻�Ƕ����
        return;
    }

    int sign = 0;  // 0: δȷ����1: ������-1: ������
    for (size_t i = 0; i < _vertices.size(); ++i) {
        const glm::vec2& p0 = _vertices[i];
        const glm::vec2& p1 = _vertices[(i + 1) % _vertices.size()];
        const glm::vec2& p2 = _vertices[(i + 2) % _vertices.size()];

        float cross = glm::cross(glm::vec3(p1 - p0, 0.f), glm::vec3(p2 - p1, 0.f)).z;  // 2D �����z ������
        if (sign == 0) {
            sign = cross > 0 ? 1 : -1;
        }
        else if (cross * sign < 0) {
            _isConvex = false;  // ���ַ�͹���
            return ;
        }
    }
    _isConvex = true;  // ���б߷���һ�£���͹�����
}

void AYPolygon2DCollider::addVertex(const glm::vec2& vertex) {
    _vertices.push_back(vertex);
    _updateConvexity();
}

void AYPolygon2DCollider::removeVertex(size_t index) {
    if (index < _vertices.size()) {
        _vertices.erase(_vertices.begin() + index);
        _updateConvexity();
    }
}

void AYPolygon2DCollider::clearVertices() {
    _vertices.clear();
    _isConvex = true;
}