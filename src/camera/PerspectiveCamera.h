#include "core/core.h"
#include "Camera.h"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera();
	PerspectiveCamera(float in_fov, float in_aspect,
		float in_near, float in_far);
	~PerspectiveCamera();

	virtual glm::mat4 getProjectionMatrix()override;
private:
	float fov;
	float aspect;
	float near;
	float far;
};
