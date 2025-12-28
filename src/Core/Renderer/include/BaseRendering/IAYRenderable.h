#pragma once
#include "AYRenderContext.h"
namespace ayt::engine::render
{
	class IAYRenderable
	{
	public:
		virtual ~IAYRenderable() = default;
		virtual void render(const AYRenderContext& context) = 0;
	};
}