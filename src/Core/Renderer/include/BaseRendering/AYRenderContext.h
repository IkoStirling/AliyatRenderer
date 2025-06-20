#pragma once
#include "AYCamera.h"
#include <memory>

struct AYRenderContext
{
	std::shared_ptr<AYCamera> camera;
};