#pragma once
#include "Mod_PhysicsSystem.h"
#include "ECPhysicsDependence.h"
#include "BasePhy/IAYPhysicsWorld.h"
#include "BasePhy/STRaycastResult.h"
#include "2DPhy/Box2DPhysicsWorld.h"

class AYPhysicsSystem : public Mod_PhysicsSystem
{
public:
	AYPhysicsSystem() = default;
	~AYPhysicsSystem() = default;

	virtual void init() override;
	virtual void update(float delta_time) override;
	virtual void shutdown() override{}

	void addToWorld(EntityID entity, WorldType type); 

	void createWorld(WorldType type);
	void destroyWorld(WorldType type);
	IAYPhysicsWorld* getPhysicsWorld(WorldType type);

	void setSimulationPaused(bool paused);
	void setFixedTimeStep(float timeStep);

	void setDebugDrawEnabled(bool enabled);
	void setDebugDrawFlags(uint32_t flags);

private:
	void _syncPhysicsToLogic();
	void _syncLogicToPhysics();
	struct PhysicsWorld {
		std::unique_ptr<IAYPhysicsWorld> impl;
		std::unordered_set<EntityID> entities;
	};
	std::unordered_map<WorldType, PhysicsWorld> _worlds;

	bool _paused = false;

	bool _isFixedPhysicsSimulater = true;
	float _fixedTimeStep = 1.0f / 60.0f;
	int _velocityIterations = 8;
	int _positionIterations = 3;

	bool _debugDrawEnabled = false;
	uint32_t _debugDrawFlags = 0;
};

REGISTER_MODULE_CLASS("PhysicsSystem", AYPhysicsSystem)