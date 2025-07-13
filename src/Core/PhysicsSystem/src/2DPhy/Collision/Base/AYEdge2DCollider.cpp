#include "2DPhy/Collision/Base/AYEdge2DCollider.h"

AYEdge2DCollider::AYEdge2DCollider(const glm::vec2& vertex1,
    const glm::vec2& vertex2) :
    _vertex1(vertex1),
    _vertex2(vertex2)
{

}

void AYEdge2DCollider::setOffset(const glm::vec2& offset)
{
    glm::vec2 center = (_vertex1 + _vertex2) * 0.5f;
    glm::vec2 delta = offset - center;
    _vertex1 += delta;
    _vertex2 += delta;
    _offset = offset;
}

float AYEdge2DCollider::getLength() const {
    return glm::distance(_vertex1, _vertex2);
}

glm::vec2 AYEdge2DCollider::getDirection() const {
    return glm::normalize(_vertex2 - _vertex1);
}

