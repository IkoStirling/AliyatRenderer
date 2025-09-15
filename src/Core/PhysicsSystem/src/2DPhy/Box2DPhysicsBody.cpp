#include "2DPhy/Box2DPhysicsBody.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"
#include "2DPhy/Collision/Box2D/Box2DCircleCollider.h"
#include "2DPhy/Collision/Box2D/Box2DPolygonCollider.h"
#include "2DPhy/Collision/Box2D/Box2DEdgeCollider.h"


Box2DPhysicsBody::Box2DPhysicsBody(b2WorldId worldId, const glm::vec2& position,
    float rotation, BodyType type) 
{
    b2BodyDef def = b2DefaultBodyDef();
    def.position = glmToBox2D(position);
    def.rotation = f2B2Rot(rotation);
    def.type = convertBodyType(type);
    def.userData = this;
    _bodyId = b2CreateBody(worldId, &def);

}

Box2DPhysicsBody::~Box2DPhysicsBody() {
    if (B2_IS_NON_NULL(_bodyId)) {
        b2DestroyBody(_bodyId);
    }
}

void Box2DPhysicsBody::setType(BodyType type)
{
    IAYPhysicsBody::setType(type);
    b2Body_SetType(_bodyId, convertBodyType(type));
}

void Box2DPhysicsBody::setTransform(const STTransform& transform) {
    b2Vec2 pos = { transform.position.x, transform.position.y };
    b2Body_SetTransform(_bodyId, pos, b2Rot(transform.rotation.getEulerAngles().z));
}

STTransform Box2DPhysicsBody::getTransform()
{
    b2Transform trans = b2Body_GetTransform(_bodyId);
    return STTransform{
        glm::vec3(trans.p.x, trans.p.y, 0),
        glm::vec3(0, 0, b2Rot_GetAngle(trans.q))
    };
}

glm::vec2 Box2DPhysicsBody::getPosition()
{
    return box2DToGlm(b2Body_GetPosition(_bodyId));
}

void Box2DPhysicsBody::setLinearVelocity(const glm::vec2& velocity) {
    b2Body_SetLinearVelocity(_bodyId, glmToBox2D(velocity));
}

glm::vec2 Box2DPhysicsBody::getLinearVelocity() const {
    b2Vec2 velocity = b2Body_GetLinearVelocity(_bodyId);
    return box2DToGlm(velocity);
}

void Box2DPhysicsBody::setAngularVelocity(float velocity) {
    b2Body_SetAngularVelocity(_bodyId, velocity);
}

float Box2DPhysicsBody::getAngularVelocity() const {
    return b2Body_GetAngularVelocity(_bodyId);
}

void Box2DPhysicsBody::applyForce(const glm::vec2& force) {
    b2Body_ApplyForceToCenter(_bodyId, glmToBox2D(force), true);
}

void Box2DPhysicsBody::applyImpulse(const glm::vec2& impulse) {
    b2Body_ApplyLinearImpulseToCenter(_bodyId, glmToBox2D(impulse), true);
}

void Box2DPhysicsBody::applyTorque(float torque) {
    b2Body_ApplyTorque(_bodyId, torque, true);
}

void Box2DPhysicsBody::addCollider(IAYCollider* collider) {
    if (!collider || _colliderShapes.count(collider)) return;

    if (b2ShapeId shapeId = _createShape(collider); B2_IS_NON_NULL(shapeId)) {
        _colliderShapes[collider] = shapeId;
    }
}

void Box2DPhysicsBody::removeCollider(IAYCollider* collider) {
    auto it = _colliderShapes.find(collider);
    if (it != _colliderShapes.end()) {
        b2DestroyShape(it->second, true);
        _colliderShapes.erase(it);
    }
}

void Box2DPhysicsBody::removeAllColliders()
{
    for (auto& [collider, shapeId] : _colliderShapes) {
        b2DestroyShape(shapeId, false);
    }
    _colliderShapes.clear();
    b2Body_ApplyMassFromShapes(_bodyId);
}

bool Box2DPhysicsBody::hasCollider(IAYCollider* collider) const
{
    return _colliderShapes.find(collider) != _colliderShapes.end();
}

// 碰撞查询
void Box2DPhysicsBody::queryOverlapArea(const glm::vec4& area) {
    // 实现AABB查询逻辑
}

// 物理材质
void Box2DPhysicsBody::setFriction(float friction) {
    for (auto& [collider, shapeId] : _colliderShapes) {
        b2Shape_SetFriction(shapeId, friction);
    }
}

void Box2DPhysicsBody::setBounciness(float bounciness) {
    for (auto& [collider, shapeId] : _colliderShapes) {
        b2Shape_SetRestitution(shapeId, bounciness);
    }
}

void Box2DPhysicsBody::setDensity(float density) {
    for (auto& [collider, shapeId] : _colliderShapes) {
        b2Shape_SetDensity(shapeId, density, false);
    }
    b2Body_ApplyMassFromShapes(_bodyId);
}

void Box2DPhysicsBody::setTrigger(bool is_trigger) {
    // Box2D 新API中不支持动态修改
}

const std::vector<IAYCollider*> Box2DPhysicsBody::getColliders() const
{
    std::vector<IAYCollider*> result;
    for (auto& [collider, shapeId] : _colliderShapes) {
        result.push_back(collider);
    }
    return result;
}


b2ShapeId Box2DPhysicsBody::_createShape(IAYCollider* collider) 
{
    if (!collider) return b2_nullShapeId;

    // 创建形状定义
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = collider->getDensity();
    shapeDef.material.friction = collider->getFriction();
    shapeDef.material.restitution = collider->getRestitution();
    shapeDef.isSensor = collider->isTrigger();

    // 碰撞过滤
    b2Filter filter;
    filter.categoryBits = collider->getCategoryBits();  //当前碰撞类别
    filter.maskBits = collider->getMaskBits();          //可碰撞的类别
    filter.groupIndex = 0;                              //巨坑
    shapeDef.filter = filter;

    b2ShapeId shapeId = b2_nullShapeId;

    // 根据碰撞体类型创建形状
    if (auto castCollider = dynamic_cast<Box2DColliderBase*>(collider))
    {
        shapeId = castCollider->createShape(_bodyId, shapeDef);
    }

    return shapeId;
}

void Box2DPhysicsBody::_updateShapeProperties(b2ShapeId shapeId, IAYCollider* collider) 
{
    if (!B2_IS_NON_NULL(shapeId) || !collider) return;

    // 更新物理材质
    b2Shape_SetFriction(shapeId, collider->getFriction());
    b2Shape_SetRestitution(shapeId, collider->getRestitution());
    b2Shape_SetDensity(shapeId, collider->getDensity(), true);

    // 更新碰撞过滤
    b2Filter filter;
    filter.categoryBits = collider->getCategoryBits();
    filter.maskBits = collider->getMaskBits();
    b2Shape_SetFilter(shapeId, filter);
}

// 类型转换
b2BodyType Box2DPhysicsBody::convertBodyType(BodyType type) {
    switch (type) {
    case BodyType::Static:    return b2_staticBody;
    case BodyType::Dynamic:   return b2_dynamicBody;
    case BodyType::Kinematic: return b2_kinematicBody;
    default:                return b2_dynamicBody;
    }
}

void Box2DPhysicsBody::setB2BodyUserData(void* userData)
{
    if (B2_IS_NON_NULL(_bodyId)) {
        b2Body_SetUserData(_bodyId, userData);
    }
}

void* Box2DPhysicsBody::getB2BodyUserData() const
{
    if (B2_IS_NON_NULL(_bodyId)) {
        return b2Body_GetUserData(_bodyId);
    }
    return nullptr;
}

glm::vec4 Box2DPhysicsBody::getAABB() const
{
    if (!B2_IS_NON_NULL(_bodyId))
        return glm::vec4(0.0f);

    b2AABB aabb = b2Body_ComputeAABB(_bodyId);

    if (aabb.lowerBound.x > aabb.upperBound.x || aabb.lowerBound.y > aabb.upperBound.y)
    {
        return glm::vec4(0.0f);
    }

    return glm::vec4(
        aabb.lowerBound.x,
        aabb.lowerBound.y,
        aabb.upperBound.x,
        aabb.upperBound.y
    );
}

std::vector<glm::vec2> Box2DPhysicsBody::getColliderVertices() const
{
    std::vector<glm::vec2> vertices;

    if (B2_IS_NULL(_bodyId)) return vertices;

    // 获取刚体的变换
    b2Transform transform = b2Body_GetTransform(_bodyId);

    // 获取所有形状
    b2ShapeId shapeArray[32];
    int shapeCount = b2Body_GetShapes(_bodyId, shapeArray, 32);

    for (int i = 0; i < shapeCount; i++)
    {
        b2ShapeId shapeId = shapeArray[i];
        b2ShapeType shapeType = b2Shape_GetType(shapeId);

        switch (shapeType)
        {
        case b2_polygonShape:
        {
            // 获取多边形数据 - 注意：返回的是结构体，不是指针
            b2Polygon polygon = b2Shape_GetPolygon(shapeId);
            for (int j = 0; j < polygon.count; j++)
            {
                // 将本地坐标转换为世界坐标
                b2Vec2 worldVertex = b2TransformPoint(transform, polygon.vertices[j]);
                vertices.emplace_back(worldVertex.x, worldVertex.y);
            }
            break;
        }
        case b2_circleShape:
        {
            // 获取圆形数据
            b2Circle circle = b2Shape_GetCircle(shapeId);

            // 圆心世界坐标
            b2Vec2 center = b2TransformPoint(transform, circle.center);

            // 为圆形添加底部点（保持原有逻辑）
            b2Vec2 bottom = { center.x, center.y - circle.radius };
            vertices.emplace_back(bottom.x, bottom.y);

            // 可选：添加更多点来更好表示圆形
            vertices.emplace_back(center.x + circle.radius, center.y); // 右
            vertices.emplace_back(center.x - circle.radius, center.y); // 左
            vertices.emplace_back(center.x, center.y + circle.radius); // 上
            break;
        }
        case b2_segmentShape:
        {
            // 获取线段数据
            b2Segment segment = b2Shape_GetSegment(shapeId);

            // 线段端点世界坐标
            b2Vec2 vertex1 = b2TransformPoint(transform, segment.point1);
            b2Vec2 vertex2 = b2TransformPoint(transform, segment.point2);

            vertices.emplace_back(vertex1.x, vertex1.y);
            vertices.emplace_back(vertex2.x, vertex2.y);
            break;
        }
        case b2_capsuleShape:
        {
            b2Capsule capsule = b2Shape_GetCapsule(shapeId);
            b2Vec2 point1 = b2TransformPoint(transform, capsule.center1);
            b2Vec2 point2 = b2TransformPoint(transform, capsule.center2);

            vertices.emplace_back(point1.x, point1.y);
            vertices.emplace_back(point2.x, point2.y);

            // 计算法线
            b2Vec2 dir = b2Sub(point2, point1);
            float length_squared = b2Dot(dir, dir);
            if (length_squared > 0.0f) {
                float inv_length = 1.0f / sqrtf(length_squared);
                dir = { dir.x * inv_length, dir.y * inv_length };
            }
            b2Vec2 normal = { -dir.y, dir.x };

            // 添加半径点
            float radius = capsule.radius;
            vertices.emplace_back(point1.x + normal.x * radius, point1.y + normal.y * radius);
            vertices.emplace_back(point1.x - normal.x * radius, point1.y - normal.y * radius);
            vertices.emplace_back(point2.x + normal.x * radius, point2.y + normal.y * radius);
            vertices.emplace_back(point2.x - normal.x * radius, point2.y - normal.y * radius);
            break;
        }
        default:
            break;
        }
    }

    return vertices;
}

glm::vec2 Box2DPhysicsBody::getLowestPoint() const
{
    std::vector<glm::vec2> vertices = getColliderVertices();
    if (vertices.empty()) return glm::vec2(0);

    glm::vec2 lowestPoint = vertices[0];
    for (const auto& vertex : vertices)
    {
        if (vertex.y < lowestPoint.y)
        {
            lowestPoint = vertex;
        }
    }

    return lowestPoint;
}

glm::vec2 Box2DPhysicsBody::getHighestPoint() const
{
    std::vector<glm::vec2> vertices = getColliderVertices();
    if (vertices.empty()) return glm::vec2(0);

    glm::vec2 highestPoint = vertices[0];
    for (const auto& vertex : vertices)
    {
        if (vertex.y > highestPoint.y)
        {
            highestPoint = vertex;
        }
    }

    return highestPoint;
}