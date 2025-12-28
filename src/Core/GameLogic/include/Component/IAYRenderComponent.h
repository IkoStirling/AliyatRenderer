#pragma once
#include "IAYComponent.h"
#include "BaseRendering/IAYRenderable.h"

namespace ayt::engine::game 
{

	class IRenderComponent : public IComponent, public ::ayt::engine::render::IRenderable
	{
	public:
		virtual void render(const ::ayt::engine::render::RenderContext& context) = 0;
	};
}