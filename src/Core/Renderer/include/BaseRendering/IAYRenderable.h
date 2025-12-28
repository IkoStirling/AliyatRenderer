#pragma once
#include "AYRenderContext.h"
namespace ayt::engine::render
{
	class IRenderable
	{
	public:
		virtual ~IRenderable() = default;
		virtual void render(const RenderContext& context) = 0;
	};
}