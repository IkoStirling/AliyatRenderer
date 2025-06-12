#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Object
{
public:
	Object(Object* _parentObj) :parentObj(_parentObj) {};

	virtual glm::vec3 getWorldLocation()
	{
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.f), modelPosition)
			* getRotationMatrix()
			* glm::scale(glm::mat4(1.f), modelScale);
		if (parentObj)
			modelMatrix = glm::translate(glm::mat4(1.f), parentObj->getWorldLocation())
			* modelMatrix;

		return glm::vec3(modelMatrix *
			glm::vec4(0.f, 0.f, 0.f, 1.f));
	}

	virtual glm::mat4 getRotationMatrix()
	{
		if (glm::length(modelRotation) > 0.f)
			return glm::rotate(
				glm::mat4(1.f),
				glm::length(modelRotation),
				glm::normalize(modelRotation)
			);
		else
			return glm::mat4(1.f);
	}

	virtual void move(glm::vec3 _move)
	{
		modelPosition =
			glm::vec3(glm::translate(glm::mat4(1.f), _move) 
				* glm::vec4(modelPosition,1.f));
	}

protected:
	Object* parentObj;
	glm::vec3 modelPosition{ 0.f,0.f,0.f };
	glm::vec3 modelScale{ 1.f,1.f,1.f };
	glm::vec3 modelRotation{ 0.f,0.f,0.f };
	//glm::vec3 worldPosition{ 0.f,0.f,0.f };
};