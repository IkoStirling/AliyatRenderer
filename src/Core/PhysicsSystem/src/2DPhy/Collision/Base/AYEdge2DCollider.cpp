#include "2DPhy/Collision/Base/AYEdge2DCollider.h"
namespace ayt::engine::physics
{
    Edge2DCollider::Edge2DCollider(const math::Vector2& vertex1,
        const math::Vector2& vertex2) :
        _vertex1(vertex1),
        _vertex2(vertex2)
    {

    }

    void Edge2DCollider::setOffset(const math::Vector2& offset)
    {
        math::Vector2 center = (_vertex1 + _vertex2) * 0.5f;
        math::Vector2 delta = offset - center;
        _vertex1 += delta;
        _vertex2 += delta;
        _offset = offset;
    }

    float Edge2DCollider::getLength() const {
        return glm::distance(_vertex1, _vertex2);
    }

    math::Vector2 Edge2DCollider::getDirection() const {
        return glm::normalize(_vertex2 - _vertex1);
    }

}