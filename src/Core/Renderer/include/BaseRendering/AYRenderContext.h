#pragma once
#include "BaseRendering/Camera/IAYCamera.h"
#include <memory>
namespace ayt::engine::render
{
	struct AYRenderContext
	{
		uint32_t currentCameraID = 0;

		void validate() const {
			assert(currentCameraID && "Camera must be set before rendering");
		}
	};
}