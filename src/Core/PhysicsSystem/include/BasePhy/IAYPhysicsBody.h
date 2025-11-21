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
    virtual void setOwningEntity(EntityID entity) { _owningObject = entity; }
    virtual EntityID getOwningEntity() { return _owningObject; }

    virtual bool isTransformDirty() { return _isDirty; };

    //---------------位置旋转----------------
    virtual void setTransform(const STTransform& transform) = 0;
    virtual STTransform getTransform() = 0;
    virtual AYMath::Vector2 getPosition() = 0;

    //-----------------速度------------------
    virtual void setLinearVelocity(const AYMath::Vector2& velocity) = 0;
    virtual AYMath::Vector2 getLinearVelocity() const = 0;
    virtual void setAngularVelocity(float velocity) = 0;
    virtual float  getAngularVelocity() const = 0;


    //---------------力和运动----------------
    virtual void applyForce(const AYMath::Vector2& force) = 0;
    virtual void applyImpulse(const AYMath::Vector2& impulse) = 0;  //冲量
    virtual void applyTorque(float torque) = 0;    //扭矩


    //---------------碰撞管理----------------
    // 碰撞体
    virtual void addCollider(IAYCollider* collider) = 0;  
    virtual void removeCollider(IAYCollider* collider) = 0;    
    virtual void setTrigger(bool is_trigger) = 0;   

    // 获取所有碰撞体
    virtual const std::vector<IAYCollider*> getColliders() const = 0;

    // 碰撞查询
    virtual void queryOverlapArea(const AYMath::Vector4& area) = 0;


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
    EntityID _owningObject = -1;
    bool _isDirty = false;
};