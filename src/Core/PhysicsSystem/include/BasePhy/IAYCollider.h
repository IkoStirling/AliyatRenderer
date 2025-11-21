#pragma once
#include "ECPhysicsDependence.h"

class IAYPhysicsBody; // 前向声明

class IAYCollider {
public:
    enum class ShapeType {
        Box2D,
        Circle2D,
        Polygon2D,
        Edge2D
    };

    virtual ~IAYCollider() = default;

    // 基础属性
    virtual ShapeType getShapeType() const = 0;
    virtual void setOffset(const AYMath::Vector2& offset) { _offset = offset; }
    virtual AYMath::Vector2 getOffset() const { return _offset; }
    virtual void setRotation(const AYMath::Vector3& rotation) { _rotation = rotation; }

    // 物理材质
    virtual void setFriction(float friction) { _friction = friction; }
    virtual void setRestitution(float restitution) { _restitution = restitution; }
    virtual void setDensity(float density) { _density = density; }
    virtual float getFriction() const { return _friction; }
    virtual float getRestitution() const { return _restitution; }
    virtual float getDensity() const { return _density; }

    // 碰撞过滤
    virtual void setCategoryBits(uint16_t bits) { _categoryBits = bits; }
    virtual void setMaskBits(uint16_t bits) { _maskBits = bits; }
    virtual uint16_t getCategoryBits() const { return _categoryBits; }
    virtual uint16_t getMaskBits() const { return _maskBits; }

    // 触发模式
    virtual void setIsTrigger(bool trigger) { _isTrigger = trigger; }
    virtual bool isTrigger() const { return _isTrigger; }

protected:
    AYMath::Vector2 _offset = { 0.0f, 0.0f };
    AYMath::Vector3 _rotation = { 0.0f, 0.0f, 0.0f };
    float _friction = 0.2f;
    float _restitution = 0.0f;
    float _density = 1.0f;
    uint16_t _categoryBits = 0x0001;
    uint16_t _maskBits = 0xFFFF;
    bool _isTrigger = false;
};