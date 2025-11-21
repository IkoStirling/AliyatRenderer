#include "2DPhy/Collision/Base/AYEdge2DCollider.h"

AYEdge2DCollider::AYEdge2DCollider(const AYMath::Vector2& vertex1,
    const AYMath::Vector2& vertex2) :
    _vertex1(vertex1),
    _vertex2(vertex2)
{

}

void AYEdge2DCollider::setOffset(const AYMath::Vector2& offset)
{
    AYMath::Vector2 center = (_vertex1 + _vertex2) * 0.5f;
    AYMath::Vector2 delta = offset - center;
    _vertex1 += delta;
    _vertex2 += delta;
    _offset = offset;
}

float AYEdge2DCollider::getLength() const {
    return glm::distance(_vertex1, _vertex2);
}

AYMath::Vector2 AYEdge2DCollider::getDirection() const {
    return glm::normalize(_vertex2 - _vertex1);
}

