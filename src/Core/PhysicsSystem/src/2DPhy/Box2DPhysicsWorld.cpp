#include "2DPhy/Box2DPhysicsWorld.h"


Box2DPhysicsWorld::Box2DPhysicsWorld()
{
    b2Vec2 gravity = { 0.0f, -9.8f };
    b2WorldDef def = b2DefaultWorldDef();
    def.gravity = gravity;
    _worldId = b2CreateWorld(&def);
}

Box2DPhysicsWorld::~Box2DPhysicsWorld()
{
    if (B2_IS_NON_NULL(_worldId)) {
        _bodies.clear();
        b2DestroyWorld(_worldId);
    }
}

void Box2DPhysicsWorld::step(float deltaTime, int velocity_iterations, int position_iterations)
{
    b2World_Step(_worldId, deltaTime, 4);
}

void Box2DPhysicsWorld::setGravity(const glm::vec3& gravity)
{
    b2Vec2 b2Gravity = { gravity.x, gravity.y };
    b2World_SetGravity(_worldId, b2Gravity);
}

bool Box2DPhysicsWorld::raycast(const glm::vec3& start, const glm::vec3& end, STRaycastResult& hit)
{
    b2Vec2 b2Start = { start.x, start.y };
    b2Vec2 b2End = { end.x, end.y };
    b2Vec2 translation = { b2End.x - b2Start.x, b2End.y - b2Start.y };

    b2QueryFilter filter = getDefaultFilter();
    b2RayResult result = b2World_CastRayClosest(_worldId, b2Start, translation, filter);

    if (result.hit)
    {
        fillRaycastResult(result, hit);
        return true;
    }

    return false;
}

bool Box2DPhysicsWorld::raycast(const glm::vec3& start, const glm::vec3& end, std::function<bool(const STRaycastResult&)> callback)
{
    b2Vec2 b2Start = { start.x, start.y };
    b2Vec2 b2End = { end.x, end.y };
    b2Vec2 translation = { b2End.x - b2Start.x, b2End.y - b2Start.y };

    b2QueryFilter filter = getDefaultFilter();

    // 对于多个命中的情况，我们需要使用更复杂的 CastRay 函数
    // 这里简化处理，只检测最近的一个命中
    b2RayResult result = b2World_CastRayClosest(_worldId, b2Start, translation, filter);

    if (result.hit)
    {
        STRaycastResult hit;
        fillRaycastResult(result, hit);
        return callback(hit);
    }

    return false;
}

IAYPhysicsBody* Box2DPhysicsWorld::createBody(EntityID entity, const glm::vec3& position, float rotation, IAYPhysicsBody::BodyType type)
{
    std::lock_guard<std::mutex> lock(_bbodyMutex);
    auto [iter, inserted] = _bodies.try_emplace(entity, std::make_unique<Box2DPhysicsBody>(_worldId, glm::vec2(position), rotation, type));
    auto& body = iter->second;
    body->setOwningEntity(entity);
    body->setType(type);

    return body.get();
}

void Box2DPhysicsWorld::setTransform(EntityID entity, const STTransform& transform)
{
    auto it = _bodies.find(entity);
    if (it != _bodies.end())
    {
        it->second->setTransform(transform);
    }
}

std::vector<IAYPhysicsBody*> Box2DPhysicsWorld::getAllBodies()
{
    std::vector<IAYPhysicsBody*> res;

    for (auto& [id, body] : _bodies)
    {
        res.push_back(body.get());
    }
    return res;
}

const STTransform& Box2DPhysicsWorld::getTransform(EntityID entity)
{
    auto it = _bodies.find(entity);
    if (it != _bodies.end())
    {
        return it->second->getTransform();
    }
    static STTransform emptyTransform;
    return emptyTransform;
}

Box2DPhysicsWorld::GroundContactInfo Box2DPhysicsWorld::checkGroundContact(IAYPhysicsBody* body, float maxDistance)
{
    GroundContactInfo info;
    info.isGrounded = false;
    info.distanceToGround = maxDistance;

    if (!body) return info;

    Box2DPhysicsBody* box2DBody = static_cast<Box2DPhysicsBody*>(body);
    b2BodyId bodyId = box2DBody->getB2BodyId();
    if (B2_ID_EQUALS(bodyId, b2_nullBodyId)) return info;

    // 获取碰撞体最低点
    glm::vec2 lowestPoint = box2DBody->getLowestPoint();

    // 向下发射射线检测地面
    b2Vec2 origin = { lowestPoint.x, lowestPoint.y + 0.015f };
    b2Vec2 translation = { 0.0f, -maxDistance };

    b2QueryFilter filter = getGroundFilter();
    b2RayResult result = b2World_CastRayClosest(_worldId, origin, translation, filter);

    if (result.hit)
    {
        info.isGrounded = true;
        info.distanceToGround = maxDistance * result.fraction;
        info.contactPoint = glm::vec2(result.point.x, result.point.y);
        info.contactNormal = glm::vec2(result.normal.x, result.normal.y);

        // 从形状获取刚体
        b2BodyId groundBodyId = b2Shape_GetBody(result.shapeId);
        info.groundBody = static_cast<IAYPhysicsBody*>(b2Body_GetUserData(groundBodyId));
    }

    return info;
}

STRaycastResult Box2DPhysicsWorld::raycastToGround(const glm::vec2& point, float maxDistance)
{
    STRaycastResult hit;

    // 向下发射射线
    b2Vec2 origin = { point.x, point.y };
    b2Vec2 translation = { 0.0f, -maxDistance };

    b2QueryFilter filter = getGroundFilter();
    b2RayResult result = b2World_CastRayClosest(_worldId, origin, translation, filter);

    if (result.hit)
    {
        fillRaycastResult(result, hit);
    }

    return hit;
}

b2QueryFilter Box2DPhysicsWorld::getDefaultFilter() const
{
    b2QueryFilter filter;
    filter.categoryBits = 0xFFFF;
    filter.maskBits = 0xFFFF;
    return filter;
}

b2QueryFilter Box2DPhysicsWorld::getGroundFilter() const
{
    b2QueryFilter filter;
    filter.categoryBits = 0x0001; // 地面类别
    filter.maskBits = 0xFFFF;     // 与所有类别碰撞
    return filter;
}

void Box2DPhysicsWorld::fillRaycastResult(const b2RayResult& result, STRaycastResult& hit)
{
    if (result.hit)
    {
        b2BodyId bodyId = b2Shape_GetBody(result.shapeId);
        hit.body = static_cast<IAYPhysicsBody*>(b2Body_GetUserData(bodyId));
        hit.point = glm::vec3(result.point.x, result.point.y, 0);
        hit.normal = glm::vec3(result.normal.x, result.normal.y, 0);
        hit.fraction = result.fraction;
    }
    else
    {
        hit.body = nullptr;
        hit.fraction = 1.0f;
    }
}