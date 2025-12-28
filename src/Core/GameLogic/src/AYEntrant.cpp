#include "AYEntrant.h"

namespace ayt::engine::game
{
	AYEntrant::AYEntrant(const std::string& name) :
		AYGameObject(name)
	{
		_physics = addComponent<AYPhysicsComponent>("_physics");
		auto ecsEngine = GET_CAST_MODULE(ECS, "ECSEngine");
		_entity = ecsEngine->createEntity();
		ecsEngine->addComponent<math::Transform>(_entity);
	}

	AYEntrant::~AYEntrant()
	{
	}

}