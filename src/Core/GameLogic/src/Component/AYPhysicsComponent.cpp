#include "Component/AYPhysicsComponent.h"
#include "AYEntrant.h" 

namespace ayt::engine::game
{
    using  namespace ayt::engine::physics;
    using  namespace ayt::engine::ecs;

    void AYPhysicsComponent::beginPlay()
    {
        auto physicsSystem = GET_CAST_MODULE(AYPhysicsSystem, "PhysicsSystem");
        auto ecsEngine = GET_CAST_MODULE(ECS, "ECSEngine");
        auto entrant = dynamic_cast<AYEntrant*>(getOwner());
        if (!entrant)
            throw std::runtime_error("[AYPhysicsComponent] object not entrant");
        _physicsBody = physicsSystem->getPhysicsWorld(WorldType::AY2D)
            ->createBody(
                entrant->getEntityID(),
                getOwner()->getPosition(),
                0.f,
                _bodyType);

        for (auto& collider : _colliders) {
            _physicsBody->addCollider(collider.get());
        }
    }

    void AYPhysicsComponent::update(float delta_time)
    {
        if (_is3D)
            return;
    }

    void AYPhysicsComponent::setBodyType(IAYPhysicsBody::BodyType type)
    {
        _bodyType = type;
        if (_physicsBody)
            _physicsBody->setType(_bodyType);
    }

    void AYPhysicsComponent::addCollider(std::shared_ptr<IAYCollider> collider)
    {
        _colliders.push_back(collider);
    }

    void AYPhysicsComponent::removeCollider(std::shared_ptr<IAYCollider> collider)
    {
        auto it = std::find(_colliders.begin(), _colliders.end(), collider);

        if (it != _colliders.end())
        {
            _colliders.erase(it);
        }
    }
}