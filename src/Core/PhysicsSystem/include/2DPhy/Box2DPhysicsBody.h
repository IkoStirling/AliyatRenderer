#pragma once
#include "BasePhy/IAYPhysicsBody.h"
#include <box2d/box2d.h>
#include <box2d/math_functions.h>

// 此类中坐标系均为Box2D世界坐标系
class Box2DPhysicsBody : public IAYPhysicsBody
{
public:
    Box2DPhysicsBody(b2WorldId worldId, const AYMath::Vector2& position, float rotation, BodyType type);
    ~Box2DPhysicsBody() override;

    // 实现IAYPhysicsBody接口
    void setType(BodyType type) override;
    void setTransform(const STTransform& transform) override;
    STTransform getTransform() override;
    AYMath::Vector2 getPosition() override;
    void setLinearVelocity(const AYMath::Vector2& velocity) override;
    AYMath::Vector2 getLinearVelocity() const override;
    void setAngularVelocity(float velocity) override;
    float getAngularVelocity() const override;
    void applyForce(const AYMath::Vector2& force) override;
    void applyImpulse(const AYMath::Vector2& impulse) override;
    void applyTorque(float torque) override;
    void addCollider(IAYCollider* collider) override;
    void removeCollider(IAYCollider* collider) override;
    void removeAllColliders();
    bool hasCollider(IAYCollider* collider) const;
    void setTrigger(bool is_trigger) override;
    const std::vector<IAYCollider*> getColliders() const override;
    void queryOverlapArea(const AYMath::Vector4& area) override;
    void setFriction(float friction) override;
    void setBounciness(float bounciness) override;
    void setDensity(float density) override;
    
    // 物理材质批量设置
    void updateAllFixtureMaterials();
    // 获取底层Box2D body（供高级操作使用）
    b2BodyId getB2BodyId() const { return _bodyId; }
    // 设置Box2D body的用户数据指针
    void setB2BodyUserData(void* userData);
    // 获取Box2D body的用户数据指针
    void* getB2BodyUserData() const;

    AYMath::Vector4 getAABB() const;
    std::vector<AYMath::Vector2> getColliderVertices() const;
    AYMath::Vector2 getLowestPoint() const;
    AYMath::Vector2 getHighestPoint() const;
private:
    b2BodyId _bodyId = b2_nullBodyId;
    std::unordered_map<IAYCollider*, b2ShapeId> _colliderShapes;
    // 内部辅助方法
    b2ShapeId _createShape(IAYCollider* collider);
    void _updateShapeProperties(b2ShapeId shapeId, IAYCollider* collider);

    static b2BodyType convertBodyType(BodyType type);
};