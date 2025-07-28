#pragma once
#include "ECPhysicsDependence.h"
#include "IAYCollider.h"
#include "BasePhy/IAYPhysical.h"

class IAYPhysicsBody
{
public:
    enum class BodyType { Static, Dynamic, Kinematic };
    virtual ~IAYPhysicsBody() = default;
    virtual void setType(BodyType type) { _type = type; }
    virtual BodyType getType() { return _type; }
    virtual bool isDynamic() { return _type == BodyType::Dynamic; }
    virtual bool isStatic() { return _type == BodyType::Static; }
    virtual bool isKinematic() { return _type == BodyType::Kinematic; }
    virtual void setPhysicalObject(IAYPhysical* obj) { _gameObject = obj; }
    virtual IAYPhysical* getPhysicalObject() { return _gameObject; }

    virtual bool isTransformDirty() { return _isDirty; };

    //---------------位置旋转----------------
    virtual void setTransform(STTransform& transform) = 0;
    virtual glm::vec2 getPosition() = 0;

    //-----------------速度------------------
    virtual void setLinearVelocity(const glm::vec2& velocity) = 0;
    virtual glm::vec2 getLinearVelocity() const = 0;
    virtual void setAngularVelocity(float velocity) = 0;
    virtual float  getAngularVelocity() const = 0;


    //---------------力和运动----------------
    virtual void applyForce(const glm::vec2& force) = 0;
    virtual void applyImpulse(const glm::vec2& impulse) = 0;  //冲量
    virtual void applyTorque(float torque) = 0;    //扭矩


    //---------------碰撞管理----------------
    //碰撞体
    virtual void addCollider(IAYCollider* collider) = 0;  
    virtual void removeCollider(IAYCollider* collider) = 0;    
    virtual void setTrigger(bool is_trigger) = 0;   

    //碰撞查询
    virtual void queryOverlapArea(const glm::vec4& area) = 0;


    //---------------物理材质----------------
    virtual void setFriction(float friction) = 0;
    virtual void setBounciness(float bounciness) = 0;
    virtual void setDensity(float density) = 0;


    //---------------关节约束----------------
    //....


    //---------------调试方法----------------
    //....

protected:
    BodyType _type = BodyType::Dynamic;
    IAYPhysical* _gameObject = nullptr;
    bool _isDirty = false;
};