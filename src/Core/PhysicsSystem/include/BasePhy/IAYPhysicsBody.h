#pragma once
#include "ECPhysicsDependence.h"
#include "IAYCollider.h"

class IAYPhysicsBody
{
public:
    enum class BodyType { Static, Dynamic, Kinematic };
    virtual ~IAYPhysicsBody() = default;
    virtual void setType(BodyType type) = 0;

    //---------------λ����ת----------------
    virtual void setTransform(const glm::vec2& position, float rotation) = 0;
    

    //-----------------�ٶ�------------------
    virtual void setLinearVelocity(const glm::vec2& velocity) = 0;
    virtual glm::vec2 getLinearVelocity() const = 0;
    virtual void setAngularVelocity(float velocity) = 0;
    virtual float  getAngularVelocity() const = 0;


    //---------------�����˶�----------------
    virtual void applyForce(const glm::vec2& force) = 0;
    virtual void applyImpulse(const glm::vec2& impulse) = 0;  //����
    virtual void applyTorque(float torque) = 0;    //Ť��


    //---------------��ײ����----------------
    //��ײ��
    virtual void addCollider(IAYCollider* collider) = 0;  
    virtual void removeCollider(IAYCollider* collider) = 0;    
    virtual void setTrigger(bool is_trigger) = 0;   

    //��ײ��ѯ
    virtual void queryOverlapArea(const glm::vec4& area) = 0;


    //---------------�������----------------
    virtual void setFriction(float friction) = 0;
    virtual void setBounciness(float bounciness) = 0;
    virtual void setDensity(float density) = 0;


    //---------------�ؽ�Լ��----------------
    //....


    //---------------���Է���----------------
    //....

};