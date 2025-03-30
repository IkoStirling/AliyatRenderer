#include "PerspectiveCamera.h"
#include "glm/gtc/matrix_transform.hpp"

PerspectiveCamera::PerspectiveCamera(Object* _parentObj) :
	Camera(_parentObj),
	fov(0),
	aspect(0),
	near(0),
	far(0)
{
}

PerspectiveCamera::PerspectiveCamera(Object* _parentObj,float in_fov, float in_aspect, float in_near, float in_far) :
	Camera(_parentObj),
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
