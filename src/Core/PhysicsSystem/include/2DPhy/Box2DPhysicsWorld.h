#pragma once
#include "BasePhy/IAYPhysicsWorld.h"
#include "Box2DPhysicsBody.h"
namespace ayt::engine::physics
{
    class Box2DPhysicsWorld : public IAYPhysicsWorld
    {
    private:

    public:
        Box2DPhysicsWorld();
        ~Box2DPhysicsWorld();

        void step(float deltaTime, int velocity_iterations = 8, int position_iterations = 3) override;

        void setGravity(const math::Vector3& gravity) override;

        bool raycast(const math::Vector3& start, const math::Vector3& end, STRaycastResult& hit);

        bool raycast(const math::Vector3& start, const math::Vector3& end,
            std::function<bool(const STRaycastResult&)> callback) override;

        IAYPhysicsBody* createBody(EntityID entity,
            const math::Vector3& position,
            float rotation,
            IAYPhysicsBody::BodyType type) override;

        void setTransform(EntityID entity, const math::Transform& transform) override;

        std::vector<IAYPhysicsBody*> getAllBodies() override;

        const math::Transform& getTransform(EntityID entity) override;

        // 检测碰撞体与地面的碰撞
        struct GroundContactInfo
        {
            bool isGrounded;            // 是否接触地面
            float distanceToGround;     // 到地面的距离
            math::Vector2 contactPoint;     // 接触点
            math::Vector2 contactNormal;    // 接触法线
            IAYPhysicsBody* groundBody; // 接触的地面物体
        };

        GroundContactInfo checkGroundContact(IAYPhysicsBody* body, float maxDistance = 1.0f);

        // 检测特定点的地面碰撞
        STRaycastResult raycastToGround(const math::Vector2& point, float maxDistance = 1.0f);

    private:
        b2WorldId _worldId = b2_nullWorldId;
        std::unordered_map<EntityID, std::unique_ptr<Box2DPhysicsBody>> _bodies;
        std::mutex _bbodyMutex;


        // 射线检测过滤器和辅助函数
        b2QueryFilter getDefaultFilter() const;
        b2QueryFilter getGroundFilter() const;

        // 从射线结果中提取信息
        void fillRaycastResult(const b2RayResult& result, STRaycastResult& hit);
    };
}