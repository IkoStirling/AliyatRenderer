#include "AYEntrant.h"

AYEntrant::AYEntrant(const std::string& name):
	AYGameObject(name)
{
	_physics = addComponent<AYPhysicsComponent>("_physics");
	auto ecsEngine = GET_CAST_MODULE(AYECSEngine, "ECSEngine");
	_entity = ecsEngine->createEntity();
	ecsEngine->addComponent<STTransform>(_entity);
}

AYEntrant::~AYEntrant()
{
}