#include "AYPhysicsSystem.h"

void AYPhysicsSystem::init()
{
	createWorld(AYPhysicsSystem::WorldType::AY2D);
	createWorld(AYPhysicsSystem::WorldType::AY3D);
}

void AYPhysicsSystem::update(float delta_time)
{
	if (_paused) return;

	if (_isFixedPhysicsSimulater) {
		_worlds[WorldType::AY2D]->step(_fixedTimeStep, _velocityIterations, _positionIterations);
	}
	else {
		_worlds[WorldType::AY2D]->step(delta_time, _velocityIterations, _positionIterations);
	}

	for (auto& [type, world] : _worlds) {
		world->syncPhysicsToLogic();
	}
}

void AYPhysicsSystem::createWorld(WorldType type)
{
	auto it = _worlds.find(type);
	if (it != _worlds.end())
		return;

	switch (type)
	{
	case AYPhysicsSystem::WorldType::AY2D:
		_worlds.emplace(AYPhysicsSystem::WorldType::AY2D, std::make_unique<Box2DPhysicsWorld>());
		break;
	case AYPhysicsSystem::WorldType::AY3D:
		break;
	default:
		break;
	}
}

void AYPhysicsSystem::destroyWorld(WorldType type)
{
	_worlds.erase(type);
}

IAYPhysicsWorld* AYPhysicsSystem::getPhysicsWorld(WorldType type)
{
	auto it = _worlds.find(type);
	if (it != _worlds.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void AYPhysicsSystem::setSimulationPaused(bool paused)
{
	_paused = paused;
}

void AYPhysicsSystem::setFixedTimeStep(float timeStep)
{
	_fixedTimeStep = timeStep;
}

void AYPhysicsSystem::setDebugDrawEnabled(bool enabled)
{
	_debugDrawEnabled = enabled;
}

void AYPhysicsSystem::setDebugDrawFlags(uint32_t flags)
{
	_debugDrawFlags = flags;
}