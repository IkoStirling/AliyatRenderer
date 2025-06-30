#pragma once
#include "BaseRendering/Camera/IAYCamera.h"
#include <memory>

struct AYRenderContext
{
	IAYCamera* currentCamera = nullptr;

	void validate() const {
		assert(currentCamera && "Camera must be set before rendering");
	}
};