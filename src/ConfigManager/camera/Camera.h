#pragma once
#include "core/core.h"
#include "glm/glm.hpp"


class Camera : public Object
{
public:
	Camera(Object* _parentObj);
	//Camera(const glm::vec3& in_position, const glm::vec3& in_up, const glm::vec3& in_right);
	~Camera();

	glm::mat4 getViewMatrix();

	void setCameraWorldPosition(const glm::vec3& in_position);
	glm::vec3 getCameraWorldPosition();

	void setCameraVectorUP(const glm::vec3& in_up);
	glm::vec3 getCameraVectorUP();

	void setCameraVectorRight(const glm::vec3& in_right);
	glm::vec3 getCameraVectorRight();

	void setCameraYawPitchView(double xpos, double ypos);

	virtual void move(glm::vec3 _move)override;
public:
	virtual glm::mat4 getProjectionMatrix();
	glm::vec3 front{ 0.f,0.f,-1.f };
	glm::vec3 right{1.f,0.f,0.f};
private:
	float yaw{ -90.f }, pitch{ 0.f };

	float pitchMaxLimit{ 89.9f };
	float pitchMinLimit{ -60.0f };

	float sensitivity{ 0.2f };

	glm::vec3 up{0.f,1.f,0.f};
};