#include "OrthographicCamera.h"
#include "glm/gtc/matrix_transform.hpp"

OrthographicCamera::OrthographicCamera() :
	left(0),
	right(0),
	top(0),
	bottom(0),
	near(0),
	far(0)
{

}

OrthographicCamera::OrthographicCamera(float in_left, float in_right, float in_top, float in_bottom, float in_near, float in_far):
	left(in_left),
	right(in_right),
	top(in_top),
	bottom(in_bottom),
	near(in_near),
	far(in_far)
{

}

OrthographicCamera::~OrthographicCamera()
{
}

glm::mat4 OrthographicCamera::getProjectionMatrix()
{
	return glm::ortho(left,right,bottom,top,near,far);
}
