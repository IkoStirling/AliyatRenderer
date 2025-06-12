#include "PerspectiveCamera.h"
#include "glm/gtc/matrix_transform.hpp"

PerspectiveCamera::PerspectiveCamera() :
	fov(0),
	aspect(0),
	near(0),
	far(0)
{
}

PerspectiveCamera::PerspectiveCamera(float in_fov, float in_aspect, float in_near, float in_far) :
	fov(in_fov),
	aspect(in_aspect),
	near(in_near),
	far(in_far)
{

}

PerspectiveCamera::~PerspectiveCamera()
{
}

glm::mat4 PerspectiveCamera::getProjectionMatrix()
{
	float degreeToFov = fov * 3.141592 / 180.f;
	return glm::perspective(degreeToFov, aspect, near, far);
}
