#include "2DPhy/Collision/Base/AYBox2DCollider.h"
namespace ayt::engine::physics
{
    Box2DCollider::Box2DCollider(const math::Vector2& size)
        : _size(size)
    {
    }
}