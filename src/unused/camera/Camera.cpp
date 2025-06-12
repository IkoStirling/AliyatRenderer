#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera():
	position({0.0f,0.0f,1.0f}),
	up({ 0.0f,1.0f,0.0f }),
	right({ 1.0f,0.0f,0.0f })
{

}

Camera::Camera(const glm::vec3& in_position, const glm::vec3& in_up, const glm::vec3& in_right)
{
	position = in_position;
	up = in_up;
	right = in_right;
}

Camera::~Camera()
{
}

glm::mat4 Camera::getViewMatrix()
{
	glm::vec3 front = glm::cross(up, right);
	glm::vec3 center = position + front;
	return glm::lookAt(position, center, up);
}

void Camera::setCameraWorldPosition(const glm::vec3& in_position)
{
	position = in_position;
}

glm::vec3 Camera::getCameraWorldPosition()
{
	return position;
}

void Camera::setCameraVectorUP(const glm::vec3& in_up)
{
	up = in_up;
}

glm::vec3 Camera::getCameraVectorUP()
{
	return up;
}

void Camera::setCameraVectorRight(const glm::vec3& in_right)
{
	right = in_right;
}

glm::vec3 Camera::getCameraVectorRight()
{
	return right;
}

glm::mat4 Camera::getProjectionMatrix()
{
	return glm::mat4();
}

