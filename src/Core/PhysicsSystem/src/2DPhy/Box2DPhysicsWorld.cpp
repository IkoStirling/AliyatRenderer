#include "2DPhy/Box2DPhysicsWorld.h"

void Box2DPhysicsWorld::step(float deltaTime, int velocity_iterations, int position_iterations)
{
    _world.Step(deltaTime, velocity_iterations, position_iterations);
}

void Box2DPhysicsWorld::setGravity(const glm::vec3& gravity)
{
    _world.SetGravity(b2Vec2(gravity.x, gravity.y));
}

bool Box2DPhysicsWorld::raycast(const glm::vec3& start, const glm::vec3& end, STRaycastResult& hit)
{
    b2Vec2 b2Start(start.x, start.y);
    b2Vec2 b2End(end.x, end.y);

    // 创建射线检测回调
    RayCastClosestCallback callback;
    _world.RayCast(&callback, b2Start, b2End);

    if (callback._hit)
    {
        void* userData = reinterpret_cast<void*>(callback._fixture->GetBody()->GetUserData().pointer);
        hit.body = static_cast<IAYPhysicsBody*>(userData);
        hit.point = glm::vec3(callback._point.x, callback._point.y, 0);
        hit.normal = glm::vec3(callback._normal.x, callback._normal.y, 0);
        hit.fraction = callback._fraction;
        return true;
    }

    return false;
}

bool Box2DPhysicsWorld::raycast(const glm::vec3& start, const glm::vec3& end, std::function<bool(const STRaycastResult&)> callback)
{
    b2Vec2 b2Start(start.x, start.y);
    b2Vec2 b2End(end.x, end.y);

    // 创建射线检测回调
    RayCastAllCallback allCallback(callback);
    _world.RayCast(&allCallback, b2Start, b2End);
    return true;
}

IAYPhysicsBody* Box2DPhysicsWorld::createBody(EntityID entity, const glm::vec3& position, float rotation, IAYPhysicsBody::BodyType type)
{
    std::lock_guard<std::mutex> lock(_bbodyMutex);
    auto [iter, inserted] = _bodies.try_emplace(entity, std::make_unique<Box2DPhysicsBody>(_world, glm::vec2(position), rotation, type));
    auto& body = iter->second;
    body->setOwningEntity(entity);
    body->setType(type);
    return body.get();
}

void Box2DPhysicsWorld::setTransform(EntityID entity, const STTransform& transform)
{
    auto& body = _bodies[entity];
    body->setTransform(transform);
}

const STTransform& Box2DPhysicsWorld::getTransform(EntityID entity)
{
    auto& body = _bodies[entity];
    return body->getTransform();
}

Box2DPhysicsWorld::GroundContactInfo Box2DPhysicsWorld::checkGroundContact(IAYPhysicsBody* body, float maxDistance)
{
    GroundContactInfo info;
    info.isGrounded = false;
    info.distanceToGround = maxDistance;

    if (!body) return info;

    Box2DPhysicsBody* box2DBody = static_cast<Box2DPhysicsBody*>(body);
    if (!box2DBody->getB2Body()) return info;

    // 获取碰撞体最低点
    glm::vec2 lowestPoint = box2DBody->getLowestPoint();

    // 向下发射射线检测地面
    glm::vec2 rayStart = lowestPoint;
    glm::vec2 rayEnd = lowestPoint + glm::vec2(0, -maxDistance);

    GroundRayCastCallback callback;
    _world.RayCast(&callback, b2Vec2(rayStart.x, rayStart.y), b2Vec2(rayEnd.x, rayEnd.y));

    if (callback._hit)
    {
        info.isGrounded = true;
        info.distanceToGround = maxDistance * callback._fraction;
        info.contactPoint = glm::vec2(callback._point.x, callback._point.y);
        info.contactNormal = glm::vec2(callback._normal.x, callback._normal.y);

        // 获取地面物体
        void* userData = reinterpret_cast<void*>(callback._fixture->GetBody()->GetUserData().pointer);
        info.groundBody = static_cast<IAYPhysicsBody*>(userData);
    }

    return info;
}

STRaycastResult Box2DPhysicsWorld::raycastToGround(const glm::vec2& point, float maxDistance)
{
    STRaycastResult hit;

    // 向下发射射线
    glm::vec2 rayStart = point;
    glm::vec2 rayEnd = point + glm::vec2(0, -maxDistance);

    GroundRayCastCallback callback;
    _world.RayCast(&callback, b2Vec2(rayStart.x, rayStart.y), b2Vec2(rayEnd.x, rayEnd.y));

    if (callback._hit)
    {
        void* userData = reinterpret_cast<void*>(callback._fixture->GetBody()->GetUserData().pointer);
        hit.body = static_cast<IAYPhysicsBody*>(userData);
        hit.point = glm::vec3(callback._point.x, callback._point.y, 0);
        hit.normal = glm::vec3(callback._normal.x, callback._normal.y, 0);
        hit.fraction = callback._fraction;
    }

    return hit;
}

float Box2DPhysicsWorld::RayCastClosestCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
{
    // 跳过传感器（触发器）
    if (fixture->IsSensor()) {
        return -1.0f; // 继续检测
    }

    _hit = true;
    _fixture = fixture;
    _point = point;
    _normal = normal;
    _fraction = fraction;

    return fraction; // 返回分数，Box2D会继续寻找更近的命中
}

float Box2DPhysicsWorld::RayCastAllCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
{
    // 跳过传感器（触发器）
    if (fixture->IsSensor()) {
        return -1.0f; // 继续检测
    }

    STRaycastResult hit;
    void* userData = reinterpret_cast<void*>(fixture->GetBody()->GetUserData().pointer);
    hit.body = static_cast<IAYPhysicsBody*>(userData);
    hit.point = glm::vec3(point.x, point.y, 0);
    hit.normal = glm::vec3(normal.x, normal.y, 0);
    hit.fraction = fraction;

    // 调用回调函数，如果返回false则停止检测
    if (_callback && !_callback(hit)) {
        return 0.0f; // 停止检测
    }

    return 1.0f; // 继续检测
}

float Box2DPhysicsWorld::GroundRayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
{
    // 跳过传感器和自身碰撞体
    if (fixture->IsSensor()) {
        return -1.0f;
    }

    // 只检测向上的法线（地面）
    if (normal.y < 0.7f) { // 法线朝上才认为是地面
        return -1.0f;
    }

    _hit = true;
    _fixture = fixture;
    _point = point;
    _normal = normal;
    _fraction = fraction;

    return fraction;
}