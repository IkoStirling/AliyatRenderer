#include "2DPhy/Box2DPhysicsBody.h"
#include "2DPhy/Collision/Base/AYBox2DCollider.h"
#include "2DPhy/Collision/Base/AYCircle2DCollider.h"
#include "2DPhy/Collision/Base/AYPolygon2DCollider.h"
#include "2DPhy/Collision/Base/AYEdge2DCollider.h"

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

// ����ת��
b2BodyType Box2DPhysicsBody::convertBodyType(BodyType type) {
    switch (type) {
    case BodyType::Static:    return b2_staticBody;
    case BodyType::Dynamic:   return b2_dynamicBody;
    case BodyType::Kinematic: return b2_kinematicBody;
    default:                return b2_dynamicBody;
    }
}

// λ��/��ת
void Box2DPhysicsBody::setTransform(const glm::vec2& position, float rotation) {
    _body->SetTransform(glmToBox2D(position), rotation);
}

// �ٶȿ���
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

// �����˶�
void Box2DPhysicsBody::applyForce(const glm::vec2& force) {
    _body->ApplyForceToCenter(glmToBox2D(force), true);
}

void Box2DPhysicsBody::applyImpulse(const glm::vec2& impulse) {
    _body->ApplyLinearImpulse(glmToBox2D(impulse), _body->GetWorldCenter(), true);
}

void Box2DPhysicsBody::applyTorque(float torque) {
    _body->ApplyTorque(torque, true);
}

// ��ײ����
void Box2DPhysicsBody::addCollider(IAYCollider* collider) {
    if (!collider || _colliderFixtures.count(collider)) return;

    if (b2Fixture* fixture = _createFixture(collider)) {
        _colliderFixtures[collider] = fixture;
        //collider->setUserData(this); // ��ѡ���洢��������
    }
}

void Box2DPhysicsBody::removeCollider(IAYCollider* collider) {
    auto it = _colliderFixtures.find(collider);
    if (it != _colliderFixtures.end()) {
        _body->DestroyFixture(it->second);
        _colliderFixtures.erase(it);
    }
}

void Box2DPhysicsBody::setTrigger(bool is_trigger) {
    // ��������fixture����isSensor��־
}

// ��ײ��ѯ
void Box2DPhysicsBody::queryOverlapArea(const glm::vec4& area) {
    // ʵ��AABB��ѯ�߼�
}

// �������
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
    _body->ResetMassData(); // �ܶ��޸ĺ���Ҫ��������
}

void Box2DPhysicsBody::setTrigger(bool is_trigger) {
    for (auto& [collider, fixture] : _colliderFixtures) {
        fixture->SetSensor(is_trigger);
    }
}

b2Fixture* Box2DPhysicsBody::_createFixture(IAYCollider* collider) {
    b2FixtureDef fixtureDef;

    // ����ͨ������
    fixtureDef.density = collider->getDensity();
    fixtureDef.friction = collider->getFriction();
    fixtureDef.restitution = collider->getRestitution();
    fixtureDef.isSensor = collider->isTrigger();

    // ��ײ����
    b2Filter filter;
    filter.categoryBits = collider->getCategoryBits();
    filter.maskBits = collider->getMaskBits();
    fixtureDef.filter = filter;

    // ������ײ�����ʹ�����״
    switch (collider->getShapeType()) {
    case IAYCollider::ShapeType::Box2D: {
        auto boxCollider = static_cast<AYBox2DCollider*>(collider);
        b2PolygonShape shape;
        shape.SetAsBox(
            boxCollider->getSize().x * 0.5f,
            boxCollider->getSize().y * 0.5f,
            glmToBox2D(boxCollider->getOffset()),
            0.0f
        );
        fixtureDef.shape = &shape;
        break;
    }
    case IAYCollider::ShapeType::Circle2D: {
        auto circleCollider = static_cast<AYCircle2DCollider*>(collider);
        b2CircleShape shape;
        shape.m_radius = circleCollider->getRadius();
        shape.m_p = glmToBox2D(circleCollider->getOffset());
        fixtureDef.shape = &shape;
        break;
    }
                                         // ������״����...
    default:
        return nullptr;
    }

    return _body->CreateFixture(&fixtureDef);
}

void Box2DPhysicsBody::_updateFixtureProperties(b2Fixture* fixture, IAYCollider* collider) {
    if (!fixture || !collider) return;

    // �����������
    fixture->SetFriction(collider->getFriction());
    fixture->SetRestitution(collider->getRestitution());
    fixture->SetDensity(collider->getDensity());
    fixture->SetSensor(collider->isTrigger());

    // ������ײ����
    b2Filter filter = fixture->GetFilterData();
    filter.categoryBits = collider->getCategoryBits();
    filter.maskBits = collider->getMaskBits();
    fixture->Refilter();
}