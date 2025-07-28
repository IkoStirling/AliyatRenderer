#pragma once
#include "AYGameObject.h"
#include "Component/AYPhysicsComponent.h"

class AYEntrant : public AYGameObject
{
public:
	AYEntrant(const std::string& name = "Entrant");
	~AYEntrant();

protected:
	AYPhysicsComponent* _physics = nullptr;
};