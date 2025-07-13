#pragma once
#include "Mod_PhysicsSystem.h"
#include "ECPhysicsDependence.h"
#include "BasePhy/IAYPhysicsBody.h"
#include "BasePhy/IAYPhysicsWorld.h"
#include "2DPhy/Box2DPhysicsWorld.h"
#include "2DPhy/Box2DPhysicsBody.h"

class AYPhysicsSystem : public Mod_PhysicsSystem
{
public:
	AYPhysicsSystem() = default;
	~AYPhysicsSystem() = default;

	virtual void init() override;
	virtual void update(float delta_time) override;

};

REGISTER_MODULE_CLASS("PhysicsSystem", AYPhysicsSystem)