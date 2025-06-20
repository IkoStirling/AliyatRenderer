#pragma once
#include "IAYComponent.h"
#include "BaseRendering/IAYRenderable.h"

class IAYRenderComponent : public IAYComponent, public IAYRenderable
{
public:
	virtual void render(const AYRenderContext& context) = 0;

};