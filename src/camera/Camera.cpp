#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "application/Application.h"

Camera::Camera(Object* _parentObj) :
	Object(_parentObj)
{

}

//Camera::Camera(const glm::vec3& in_position, const glm::vec3& in_up, const glm::vec3& in_right)
//{
//	position = in_position;
//	up = in_up;
//	right = in_right;
//}

Camera::~Camera()
{
}

glm::mat4 Camera::getViewMatrix()
{
	glm::vec3 front = glm::cross(up, right);
	glm::vec3 center = getWorldLocation() + front;
	return glm::lookAt(getWorldLocation(), center, up);
}

void Camera::setCameraWorldPosition(const glm::vec3& in_position)
{
	modelPosition = in_position;
}

glm::vec3 Camera::getCameraWorldPosition()
{
	std::cout << parentObj->getWorldLocation().x << " "
		<< parentObj->getWorldLocation().y << " "
		<< parentObj->getWorldLocation().z << " "
		<< std::endl;
	return getWorldLocation();
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

void Camera::setCameraYawPitchView(double xpos, double ypos)
{
	int centerX, centerY;
	GLFWwindow* w = app.getWindowInstance();
	glfwGetWindowSize(w, &centerX, &centerY);
	centerX /= 2;
	centerY /= 2;

	if (xpos != centerX || ypos != centerY)
	{
		double delterX, delterY;
		delterX = xpos - centerX;
		delterY = ypos - centerY;
		yaw += delterX * sensitivity;
		pitch -= delterY * sensitivity;

		if (pitch > pitchMaxLimit)
			pitch = pitchMaxLimit;
		if (pitch < pitchMinLimit)
			pitch = pitchMinLimit;

		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(front);
		right = glm::normalize(glm::cross(front, { 0.0f,1.0f,0.0f }));
		up = glm::normalize(glm::cross(right, front));
	}

	glfwSetCursorPos(w, centerX, centerY);
}

void Camera::move(glm::vec3 _move)
{
	modelPosition =
		glm::vec3(
			glm::translate(glm::mat4(1.f), _move)
			* glm::vec4(modelPosition, 1.f));
}

glm::mat4 Camera::getProjectionMatrix()
{
	return glm::mat4();
}

