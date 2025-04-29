#pragma once
#include "core/core.h"
#include "glm/glm.hpp"

class Camera
{
public:
	Camera();
	Camera(const glm::vec3& in_position, const glm::vec3& in_up, const glm::vec3& in_right);
	~Camera();

	glm::mat4 getViewMatrix();
	
	void setCameraWorldPosition(const glm::vec3& in_position);
	glm::vec3 getCameraWorldPosition();

	void setCameraVectorUP(const glm::vec3& in_up);
	glm::vec3 getCameraVectorUP();
	
	void setCameraVectorRight(const glm::vec3& in_right);
	glm::vec3 getCameraVectorRight();


public:
	virtual glm::mat4 getProjectionMatrix();

	float yaw{ -90.f }, pitch{0.f};
private:
	glm::vec3 position;
	glm::vec3 up;
	glm::vec3 right;
};