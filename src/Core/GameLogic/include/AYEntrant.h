#pragma once
#include "AYGameObject.h"
#include "Component/AYPhysicsComponent.h"

class AYEntrant : public AYGameObject
{
public:
	AYEntrant(const std::string& name = "Entrant");
	~AYEntrant();

	virtual void beginPlay() override
	{
		AYGameObject::beginPlay();
	}

	virtual const STTransform& getTransform() const override 
	{
		return GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity); 
	}
	void setTransform(const STTransform& transform) override 
	{ 
		GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity) = transform; 
	}

	virtual glm::vec3 getPosition() const override 
	{ 
		return GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).position;
	}
	virtual void setPosition(const glm::vec3& position) override 
	{
		GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).position = position;
	}
	virtual void setPosition(const glm::vec2& position) override
	{
		GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).position = glm::vec3(position, _transform.position.z);
	}

	virtual glm::vec3 getRotation() const override
	{ 
		return GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).rotation.getEulerAngles();
	}
	virtual void setRotation(const glm::vec3& rotation) override 
	{
		GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).rotation.setEulerAngles(rotation);
	}

	virtual glm::vec3 getScale() const override 
	{ 
		return GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).scale;
	}
	virtual void setScale(const glm::vec3& scale) override
	{
		GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).scale = scale;
	}
	virtual void setScale(const glm::vec2& scale) override
	{ 
		GET_CAST_MODULE(AYECSEngine, "ECSEngine")->getComponent<STTransform>(_entity).scale = glm::vec3(scale, _transform.scale.z);
	}

	EntityID getEntityID() { return _entity; }
protected:
	AYPhysicsComponent* _physics = nullptr;
	EntityID _entity;
};