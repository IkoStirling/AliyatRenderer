#include "AYPhysicsSystem.h"

void AYPhysicsSystem::init()
{
	_2Dworld = std::make_unique<Box2DPhysicsWorld>();
}

void AYPhysicsSystem::update(float delta_time)
{
}
