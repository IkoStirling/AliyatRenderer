#include "AYEntrant.h"

AYEntrant::AYEntrant(const std::string& name):
	AYGameObject(name)
{
	_physics = addComponent<AYPhysicsComponent>("_physics");
}

AYEntrant::~AYEntrant()
{
}