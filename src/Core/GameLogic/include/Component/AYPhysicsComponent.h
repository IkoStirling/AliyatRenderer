#pragma once
#include "IAYComponent.h"
#include "AYPhysicsSystem.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"

namespace ayt::engine::game
{
    using ::ayt::engine::physics::IPhysicsBody;
    using ::ayt::engine::physics::ICollider;

    class PhysicsComponent : public IComponent
    {
    public:
        virtual void beginPlay() override;

        virtual void update(float delta_time) override;

        virtual void endPlay() override {}

        // 基础物理控制
        IPhysicsBody* getPhysicsBody() { return _physicsBody; }

        void setBodyType(IPhysicsBody::BodyType type);


        void addCollider(std::shared_ptr<ICollider> collider);


        void removeCollider(std::shared_ptr<ICollider> collider);

        void setPhysicsMode(bool is3D) { _is3D = is3D; }

    protected:
        IPhysicsBody* _physicsBody = nullptr;
        IPhysicsBody::BodyType _bodyType;
        std::vector<std::shared_ptr<ICollider>> _colliders;
        bool _is3D = false;


    };
}