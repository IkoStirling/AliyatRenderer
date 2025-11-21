#pragma once
#include "ECPhysicsDependence.h"
#include "STRaycastResult.h"
#include "IAYPhysicsBody.h"

class IAYPhysicsWorld
{
public:
    virtual ~IAYPhysicsWorld() = default;

    virtual void step(float delta_time, int velocity_iterations = 8, int position_iterations = 3) = 0;

    virtual void setGravity(const AYMath::Vector3& gravity) = 0;

    // 单点射线检测：返回最近的命中结果
    virtual bool raycast(const AYMath::Vector3& start, const AYMath::Vector3& end, STRaycastResult& hit) = 0;

    // 多点射线检测：通过回调函数返回所有命中结果，可通过返回false提前终止
    virtual bool raycast(const AYMath::Vector3& start, const AYMath::Vector3& end,
        std::function<bool(const STRaycastResult&)> callback) = 0;

    virtual IAYPhysicsBody* createBody(EntityID entity,
        const AYMath::Vector3& position = AYMath::Vector3(0),
        float rotation = 0,
        IAYPhysicsBody::BodyType type = IAYPhysicsBody::BodyType::Dynamic) = 0;
    
    virtual std::vector<IAYPhysicsBody*> getAllBodies() = 0;

    virtual void setTransform(EntityID entity, const STTransform& transform) = 0;
    virtual const STTransform& getTransform(EntityID entity) = 0;
};