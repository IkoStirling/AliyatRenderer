#pragma once
#include "BasePhy/IAYCollider.h"

class AYBox2DCollider : public IAYCollider 
{
public:
    explicit AYBox2DCollider(const glm::vec2& size = { 1.0f, 1.0f });

    // ��״����
    ShapeType getShapeType() const override { return ShapeType::Box2D; }

    // ���з���
    virtual const glm::vec2& getSize() const { return _size; }
    virtual void setSize(const glm::vec2& size) { _size = size; }


protected:
    glm::vec2 _size;
};