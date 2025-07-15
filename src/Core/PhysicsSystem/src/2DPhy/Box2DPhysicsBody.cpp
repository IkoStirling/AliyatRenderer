#include "2DPhy/Box2DPhysicsBody.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"
#include "2DPhy/Collision/Box2D/Box2DCircleCollider.h"
#include "2DPhy/Collision/Box2D/Box2DPolygonCollider.h"
#include "2DPhy/Collision/Box2D/Box2DEdgeCollider.h"


Box2DPhysicsBody::Box2DPhysicsBody(b2World& world, const glm::vec2& position,
    float rotation, BodyType type) {
    b2BodyDef def;
    def.position = glmToBox2D(position);
    def.angle = rotation;
    def.type = convertBodyType(type);
    _body = world.CreateBody(&def);
}

Box2DPhysicsBody::~Box2DPhysicsBody() {
    if (_body) {
        _body->GetWorld()->DestroyBody(_body);
    }
}

void Box2DPhysicsBody::setType(BodyType type)
{
    IAYPhysicsBody::setType(type);
    _body->SetType(convertBodyType(type));
}

void Box2DPhysicsBody::setTransform(const glm::vec2& position, float rotation) {
    _body->SetTransform(glmToBox2D(position), rotation);
}

glm::vec2 Box2DPhysicsBody::getPosition()
{
    auto& trans = _body->GetTransform();
    return box2DToGlm(trans.p);
}

void Box2DPhysicsBody::setLinearVelocity(const glm::vec2& velocity) {
    _body->SetLinearVelocity(glmToBox2D(velocity));
}

glm::vec2 Box2DPhysicsBody::getLinearVelocity() const {
    return box2DToGlm(_body->GetLinearVelocity());
}

void Box2DPhysicsBody::setAngularVelocity(float velocity) {
    _body->SetAngularVelocity(velocity);
}

float Box2DPhysicsBody::getAngularVelocity() const {
    return _body->GetAngularVelocity();
}

void Box2DPhysicsBody::applyForce(const glm::vec2& force) {
    _body->ApplyForceToCenter(glmToBox2D(force), true);
}

void Box2DPhysicsBody::applyImpulse(const glm::vec2& impulse) {
    _body->ApplyLinearImpulse(glmToBox2D(impulse), _body->GetWorldCenter(), true);
}

void Box2DPhysicsBody::applyTorque(float torque) {
    _body->ApplyTorque(torque, true);
}

void Box2DPhysicsBody::addCollider(IAYCollider* collider) {
    if (!collider || _colliderFixtures.count(collider)) return;

    if (b2Fixture* fixture = _createFixture(collider)) {
        _colliderFixtures[collider] = fixture;
        //collider->setUserData(this); // 可选：存储反向引用
    }
}

void Box2DPhysicsBody::removeCollider(IAYCollider* collider) {
    auto it = _colliderFixtures.find(collider);
    if (it != _colliderFixtures.end()) {
        _body->DestroyFixture(it->second);
        _colliderFixtures.erase(it);
    }
}

void Box2DPhysicsBody::removeAllColliders()
{
    std::unordered_map<IAYCollider*, b2Fixture*> emptyMap;

    _colliderFixtures.swap(emptyMap);

    for (auto& pair : emptyMap) {
        _body->DestroyFixture(pair.second);
    }
}

bool Box2DPhysicsBody::hasCollider(IAYCollider* collider) const
{
    return _colliderFixtures.find(collider) != _colliderFixtures.end();
}

// 碰撞查询
void Box2DPhysicsBody::queryOverlapArea(const glm::vec4& area) {
    // 实现AABB查询逻辑
}

// 物理材质
void Box2DPhysicsBody::setFriction(float friction) {
    for (auto& [collider, fixture] : _colliderFixtures) {
        fixture->SetFriction(friction);
    }
}

void Box2DPhysicsBody::setBounciness(float bounciness) {
    for (auto& [collider, fixture] : _colliderFixtures) {
        fixture->SetRestitution(bounciness);
    }
}

void Box2DPhysicsBody::setDensity(float density) {
    for (auto& [collider, fixture] : _colliderFixtures) {
        fixture->SetDensity(density);
    }
    _body->ResetMassData(); // 密度修改后需要重置质量
}

void Box2DPhysicsBody::setTrigger(bool is_trigger) {
    for (auto& [collider, fixture] : _colliderFixtures) {
        fixture->SetSensor(is_trigger);
    }
}

b2Fixture* Box2DPhysicsBody::_createFixture(IAYCollider* collider) {
    b2FixtureDef fixtureDef;    //配置模板，通过_body->CreateFixture(&fixtureDef)创建fixture

    // 设置通用属性
    fixtureDef.density = collider->getDensity();
    fixtureDef.friction = collider->getFriction();
    fixtureDef.restitution = collider->getRestitution();
    fixtureDef.isSensor = collider->isTrigger();

    // 碰撞过滤
    b2Filter filter;
    filter.categoryBits = collider->getCategoryBits();  //当前碰撞类别
    filter.maskBits = collider->getMaskBits();          //可碰撞的类别
    fixtureDef.filter = filter;

    // 根据碰撞体类型创建形状
    switch (collider->getShapeType()) {
    case IAYCollider::ShapeType::Box2D: {
        auto boxCollider = static_cast<Box2DBoxCollider*>(collider);
        fixtureDef.shape = boxCollider->createBox2DShape();
        break;
    }
    case IAYCollider::ShapeType::Circle2D: {
        auto circleCollider = static_cast<Box2DCircleCollider*>(collider);
        fixtureDef.shape = circleCollider->createBox2DShape();
        break;
    }
    case IAYCollider::ShapeType::Polygon2D: {
        auto polygonCollider = static_cast<Box2DPolygonCollider*>(collider);
        fixtureDef.shape = polygonCollider->createBox2DShape();
        break;
    }
    case IAYCollider::ShapeType::Edge2D: {
        auto edgeCollider = static_cast<Box2DEdgeCollider*>(collider);
        fixtureDef.shape = edgeCollider->createBox2DShape();
        break;
    }
    default:
        return nullptr;
    }

    return _body->CreateFixture(&fixtureDef);
}

void Box2DPhysicsBody::_updateFixtureProperties(b2Fixture* fixture, IAYCollider* collider) {
    if (!fixture || !collider) return;

    // 更新物理材质
    fixture->SetFriction(collider->getFriction());
    fixture->SetRestitution(collider->getRestitution());
    fixture->SetDensity(collider->getDensity());
    fixture->SetSensor(collider->isTrigger());

    // 更新碰撞过滤
    b2Filter filter = fixture->GetFilterData();
    filter.categoryBits = collider->getCategoryBits();
    filter.maskBits = collider->getMaskBits();
    fixture->Refilter();
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