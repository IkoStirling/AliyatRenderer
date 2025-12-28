#include "AYEntrant.h"

namespace ayt::engine::game
{
	Entrant::Entrant(const std::string& name) :
		GameObject(name)
	{
		_physics = addComponent<PhysicsComponent>("_physics");
		auto ecsEngine = GET_CAST_MODULE(ECS, "ECSEngine");
		_entity = ecsEngine->createEntity();
		ecsEngine->addComponent<math::Transform>(_entity);
	}

	Entrant::~Entrant()
	{
	}

}