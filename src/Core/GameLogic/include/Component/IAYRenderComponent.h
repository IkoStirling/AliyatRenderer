#pragma once
#include "IAYComponent.h"
#include "BaseRendering/IAYRenderable.h"

namespace ayt::engine::game 
{

	class IAYRenderComponent : public IAYComponent, public ::ayt::engine::render::IAYRenderable
	{
	public:
		virtual void render(const ::ayt::engine::render::AYRenderContext& context) = 0;
	};
}