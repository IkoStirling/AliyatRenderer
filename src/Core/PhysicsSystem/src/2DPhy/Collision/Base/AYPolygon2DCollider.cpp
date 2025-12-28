#include "2DPhy/Collision/Base/AYPolygon2DCollider.h"
namespace ayt::engine::physics
{
    Polygon2DCollider::Polygon2DCollider(const std::vector<math::Vector2>& vertices)
        : _vertices(vertices) {
        _updateConvexity();
    }

    void Polygon2DCollider::setVertices(const std::vector<math::Vector2>& vertices) {
        _vertices = vertices;
        _updateConvexity();
    }

    void Polygon2DCollider::_updateConvexity() {
        if (_vertices.size() < 3) {
            _isConvex = false;  // 少于3个顶点不是多边形
            return;
        }

        int sign = 0;  // 0: 未确定，1: 正方向，-1: 负方向
        for (size_t i = 0; i < _vertices.size(); ++i) {
            const math::Vector2& p0 = _vertices[i];
            const math::Vector2& p1 = _vertices[(i + 1) % _vertices.size()];
            const math::Vector2& p2 = _vertices[(i + 2) % _vertices.size()];

            float cross = glm::cross(math::Vector3(p1 - p0, 0.f), math::Vector3(p2 - p1, 0.f)).z;  // 2D 叉积（z 分量）
            if (sign == 0) {
                sign = cross > 0 ? 1 : -1;
            }
            else if (cross * sign < 0) {
                _isConvex = false;  // 发现非凸情况
                return;
            }
        }
        _isConvex = true;  // 所有边方向一致，是凸多边形
    }

    void Polygon2DCollider::addVertex(const math::Vector2& vertex) {
        _vertices.push_back(vertex);
        _updateConvexity();
    }

    void Polygon2DCollider::removeVertex(size_t index) {
        if (index < _vertices.size()) {
            _vertices.erase(_vertices.begin() + index);
            _updateConvexity();
        }
    }

    void Polygon2DCollider::clearVertices() {
        _vertices.clear();
        _isConvex = true;
    }
}