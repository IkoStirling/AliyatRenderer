#include "AYPhysicsSystem.h"

void AYPhysicsSystem::init()
{
	createWorld(WorldType::AY2D);
	createWorld(WorldType::AY3D);
}

void AYPhysicsSystem::update(float delta_time)
{
	if (_paused) return;

	_syncLogicToPhysics();


	for (auto& [type, world] : _worlds)
	{
		world.impl->step(_isFixedPhysicsSimulater? _fixedTimeStep: delta_time, _velocityIterations, _positionIterations);
	}

	_syncPhysicsToLogic();
}

void AYPhysicsSystem::addToWorld(EntityID entity, WorldType type)
{
	auto& world = _worlds[type];
	if (world.impl->createBody(entity)) {
		world.entities.insert(entity);
	}
}

void AYPhysicsSystem::createWorld(WorldType type)
{
	auto it = _worlds.find(type);
	if (it != _worlds.end())
		return;

	switch (type)
	{
	case WorldType::AY2D:
		_worlds.emplace(WorldType::AY2D, PhysicsWorld{ std::make_unique<Box2DPhysicsWorld>() });
		break;
	case WorldType::AY3D:
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
		return it->second.impl.get();
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

void AYPhysicsSystem::_syncLogicToPhysics()
{
	auto view = GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getView<STTransform, STPhysicsComponent>();

	for (auto [entity, transform, physics] : view)
	{
		_worlds[physics.worldType].impl->setTransform(entity, transform);
	}
}

void AYPhysicsSystem::_syncPhysicsToLogic()
{
	auto view = GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getView<STTransform, STPhysicsComponent>();

	for (auto [entity, transform, physics] : view)
	{
		auto trans = _worlds[physics.worldType].impl->getTransform(entity);
		physics.lastTransform = transform;
		transform = trans;
	}

	// 测试用
	auto view2 = GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getView<STTransform>();
	for (auto [entity, transform] : view2)
	{
		auto trans = _worlds[WorldType::AY2D].impl->getTransform(entity);
		transform = trans;
	}
}