#pragma once
#include "BasePhy/IAYPhysicsBody.h"
#include <box2d/box2d.h>

class Box2DPhysicsBody : public IAYPhysicsBody
{
public:
    Box2DPhysicsBody(b2World& world, const glm::vec2& position, float rotation, BodyType type);
    ~Box2DPhysicsBody() override;

    // ʵ��IAYPhysicsBody�ӿ�
    void setType(BodyType type) override;
    void setTransform(const glm::vec2& position, float rotation) override;
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
    void queryOverlapArea(const glm::vec4& area) override;
    void setFriction(float friction) override;
    void setBounciness(float bounciness) override;
    void setDensity(float density) override;

    
    // ���������������
    void updateAllFixtureMaterials();
    // ��ȡ�ײ�Box2D body�����߼�����ʹ�ã�
    b2Body* getB2Body() const { return _body; }
private:
    b2Body* _body;
    std::unordered_map<IAYCollider*, b2Fixture*> _colliderFixtures;
    // �ڲ���������
    b2Fixture* _createFixture(IAYCollider* collider);
    void _updateFixtureProperties(b2Fixture* fixture, IAYCollider* collider);


    static b2BodyType convertBodyType(BodyType type);
};