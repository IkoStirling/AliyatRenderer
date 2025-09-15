#pragma once
#include "BasePhy/IAYPhysicsBody.h"
#include <box2d/box2d.h>
#include <box2d/math_functions.h>

// 此类中坐标系均为Box2D世界坐标系
class Box2DPhysicsBody : public IAYPhysicsBody
{
public:
    Box2DPhysicsBody(b2WorldId worldId, const glm::vec2& position, float rotation, BodyType type);
    ~Box2DPhysicsBody() override;

    // 实现IAYPhysicsBody接口
    void setType(BodyType type) override;
    void setTransform(const STTransform& transform) override;
    STTransform getTransform() override;
    glm::vec2 getPosition() override;
    void setLinearVelocity(const glm::vec2& velocity) override;
    glm::vec2 getLinearVelocity() const override;
    void setAngularVelocity(float velocity) override;
    float getAngularVelocity() const override;
    void applyForce(const glm::vec2& force) override;
    void applyImpulse(const glm::vec2& impulse) override;
    void applyTorque(float torque) override;
    void addCollider(IAYCollider* collider) override;
    void removeCollider(IAYCollider* collider) override;
    void removeAllColliders();
    bool hasCollider(IAYCollider* collider) const;
    void setTrigger(bool is_trigger) override;
    const std::vector<IAYCollider*> getColliders() const override;
    void queryOverlapArea(const glm::vec4& area) override;
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

    glm::vec4 getAABB() const;
    std::vector<glm::vec2> getColliderVertices() const;
    glm::vec2 getLowestPoint() const;
    glm::vec2 getHighestPoint() const;
private:
    b2BodyId _bodyId = b2_nullBodyId;
    std::unordered_map<IAYCollider*, b2ShapeId> _colliderShapes;
    // 内部辅助方法
    b2ShapeId _createShape(IAYCollider* collider);
    void _updateShapeProperties(b2ShapeId shapeId, IAYCollider* collider);

    static b2BodyType convertBodyType(BodyType type);
};