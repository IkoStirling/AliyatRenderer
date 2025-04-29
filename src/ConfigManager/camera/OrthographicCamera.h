#include "core/core.h"
#include "Camera.h"

class OrthographicCamera : public Camera
{
public:
	OrthographicCamera(Object* _parentObj);
	OrthographicCamera(Object* _parentObj,float in_left, float  in_right,
		float  in_top, float  in_bottom,
		float  in_near, float  in_far);
	~OrthographicCamera();

	virtual glm::mat4 getProjectionMatrix()override;
private:
	float left;
	float right;
	float top;
	float bottom;
	float near;
	float far;
};