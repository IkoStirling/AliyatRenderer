#pragma once
#include "BasePhy/IAYPhysicsWorld.h"
#include "Box2DPhysicsBody.h"

class Box2DPhysicsWorld : public IAYPhysicsWorld
{
private:

public:
    Box2DPhysicsWorld() : _world(b2Vec2(0, -9.8f)) {}

    void step(float deltaTime, int velocity_iterations = 8, int position_iterations = 3) override;

    void setGravity(const glm::vec3& gravity) override;

    bool raycast(const glm::vec3& start, const glm::vec3& end, STRaycastResult& hit);

    bool raycast(const glm::vec3& start, const glm::vec3& end,
        std::function<bool(const STRaycastResult&)> callback) override;

    IAYPhysicsBody* createBody(EntityID entity,
        const glm::vec3& position,
        float rotation,
        IAYPhysicsBody::BodyType type) override;

    void setTransform(EntityID entity, const STTransform& transform) override;

    const STTransform& getTransform(EntityID entity) override;

    // 检测碰撞体与地面的碰撞
    struct GroundContactInfo
    {
        bool isGrounded;            // 是否接触地面
        float distanceToGround;     // 到地面的距离
        glm::vec2 contactPoint;     // 接触点
        glm::vec2 contactNormal;    // 接触法线
        IAYPhysicsBody* groundBody; // 接触的地面物体
    };

    GroundContactInfo checkGroundContact(IAYPhysicsBody* body, float maxDistance = 1.0f);

    // 检测特定点的地面碰撞
    STRaycastResult raycastToGround(const glm::vec2& point, float maxDistance = 1.0f);

private:
    b2World _world;
    std::unordered_map<EntityID, std::unique_ptr<Box2DPhysicsBody>> _bodies;
    std::mutex _bbodyMutex;

    class RayCastClosestCallback : public b2RayCastCallback
    {
    public:
        RayCastClosestCallback() : _fixture(nullptr), _hit(false) {}

        float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
            const b2Vec2& normal, float fraction) override;

        bool _hit;
        b2Fixture* _fixture;
        b2Vec2 _point;
        b2Vec2 _normal;
        float _fraction;
    };

    // Box2D射线检测回调类 - 所有命中
    class RayCastAllCallback : public b2RayCastCallback
    {
    public:
        RayCastAllCallback(std::function<bool(const STRaycastResult&)> callback)
            : _callback(callback) {
        }

        float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
            const b2Vec2& normal, float fraction) override;

        std::function<bool(const STRaycastResult&)> _callback;
    };

    class GroundRayCastCallback : public b2RayCastCallback
    {
    public:
        GroundRayCastCallback() : _fixture(nullptr), _hit(false){}

        float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
            const b2Vec2& normal, float fraction) override;

        bool _hit;
        b2Fixture* _fixture;
        b2Vec2 _point;
        b2Vec2 _normal;
        float _fraction;
    };
};