#pragma once
#include "AYGameObject.h"
#include "Component/AYPhysicsComponent.h"
namespace ayt::engine::game
{
	using ::ayt::engine::ecs::ECS;

	class Entrant : public GameObject
	{
	public:
		Entrant(const std::string& name = "Entrant");
		~Entrant();

		virtual void beginPlay() override
		{
			GameObject::beginPlay();
		}

		virtual const math::Transform& getTransform() const override
		{
			return GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity);
		}
		void setTransform(const math::Transform& transform) override
		{
			GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity) = transform;
		}

		virtual glm::vec3 getPosition() const override
		{
			return GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).position;
		}
		virtual void setPosition(const glm::vec3& position) override
		{
			GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).position = position;
		}
		virtual void setPosition(const glm::vec2& position) override
		{
			GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).position = glm::vec3(position, _transform.position.z);
		}

		virtual glm::vec3 getRotation() const override
		{
			return GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).rotation.getEulerAngles();
		}
		virtual void setRotation(const glm::vec3& rotation) override
		{
			GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).rotation.setEulerAngles(rotation);
		}

		virtual glm::vec3 getScale() const override
		{
			return GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).scale;
		}
		virtual void setScale(const glm::vec3& scale) override
		{
			GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).scale = scale;
		}
		virtual void setScale(const glm::vec2& scale) override
		{
			GET_CAST_MODULE(ECS, "ECSEngine")->getComponent<math::Transform>(_entity).scale = glm::vec3(scale, _transform.scale.z);
		}

		EntityID getEntityID() { return _entity; }
	protected:
		PhysicsComponent* _physics = nullptr;
		EntityID _entity;
	};
}