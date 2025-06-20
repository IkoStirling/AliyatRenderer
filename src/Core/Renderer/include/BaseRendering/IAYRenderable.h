#pragma once
#include "AYRenderContext.h"

class IAYRenderable
{
public:
	virtual ~IAYRenderable() = default;
	virtual void render(const AYRenderContext& context) = 0;
};