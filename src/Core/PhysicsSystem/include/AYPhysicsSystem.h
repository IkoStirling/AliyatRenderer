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

	enum class WorldType { AY2D, AY3D};
	void createWorld(WorldType type);
	void destroyWorld(WorldType type);
	IAYPhysicsWorld* getPhysicsWorld(WorldType type);

	void setSimulationPaused(bool paused);
	void setFixedTimeStep(float timeStep);

	void setDebugDrawEnabled(bool enabled);
	void setDebugDrawFlags(uint32_t flags);

private:
	std::unordered_map<WorldType, std::unique_ptr<IAYPhysicsWorld>> _worlds;

	bool _paused = false;

	bool _isFixedPhysicsSimulater = true;
	float _fixedTimeStep = 1.0f / 60.0f;
	int _velocityIterations = 8;
	int _positionIterations = 3;

	bool _debugDrawEnabled = false;
	uint32_t _debugDrawFlags = 0;
};

REGISTER_MODULE_CLASS("PhysicsSystem", AYPhysicsSystem)