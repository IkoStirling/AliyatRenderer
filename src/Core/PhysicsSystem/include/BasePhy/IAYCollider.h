#pragma once
#include "ECPhysicsDependence.h"

class IAYPhysicsBody; // ǰ������

class IAYCollider {
public:
    enum class ShapeType {
        Box2D,
        Circle2D,
        Polygon2D,
        Edge2D
    };

    virtual ~IAYCollider() = default;

    // ��������
    virtual ShapeType getShapeType() const = 0;
    virtual void setOffset(const glm::vec2& offset) { _offset = offset; }
    virtual glm::vec2 getOffset() const { return _offset; }


    // �������
    virtual void setFriction(float friction) { _friction = friction; }
    virtual void setRestitution(float restitution) { _restitution = restitution; }
    virtual void setDensity(float density) { _density = density; }
    virtual float getFriction() const { return _friction; }
    virtual float getRestitution() const { return _restitution; }
    virtual float getDensity() const { return _density; }

    // ��ײ����
    virtual void setCategoryBits(uint16_t bits) { _categoryBits = bits; }
    virtual void setMaskBits(uint16_t bits) { _maskBits = bits; }
    virtual uint16_t getCategoryBits() const { return _categoryBits; }
    virtual uint16_t getMaskBits() const { return _maskBits; }

    // ����ģʽ
    virtual void setIsTrigger(bool trigger) { _isTrigger = trigger; }
    virtual bool isTrigger() const { return _isTrigger; }

protected:
    glm::vec2 _offset = { 0.0f, 0.0f };
    float _friction = 0.2f;
    float _restitution = 0.0f;
    float _density = 1.0f;
    uint16_t _categoryBits = 0x0001;
    uint16_t _maskBits = 0xFFFF;
    bool _isTrigger = false;
};