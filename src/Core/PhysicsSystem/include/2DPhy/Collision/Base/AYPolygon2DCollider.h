#pragma once
#include "BasePhy/IAYCollider.h"
namespace ayt::engine::physics
{
    class AYPolygon2DCollider : public IAYCollider {
    public:
        explicit AYPolygon2DCollider(const std::vector<math::Vector2>& vertices = {});

        // 形状类型
        ShapeType getShapeType() const override { return ShapeType::Polygon2D; }

        // 特有方法
        const std::vector<math::Vector2>& getVertices() const { return _vertices; }
        virtual void setVertices(const std::vector<math::Vector2>& vertices);

        bool isConvex() const { return _isConvex; }
        void setConvex(bool convex) { _isConvex = convex; }

        // 实用方法
        virtual void addVertex(const math::Vector2& vertex);
        virtual void removeVertex(size_t index);
        virtual void clearVertices();

    protected:
        std::vector<math::Vector2> _vertices;
        bool _isConvex = true; // 默认为凸多边形

        void _updateConvexity(); // 自动计算凸性
    };
}