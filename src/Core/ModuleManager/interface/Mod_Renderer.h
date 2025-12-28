#pragma once
#include "IAYModule.h"
#include <functional>
namespace ayt::engine::render
{
	class RenderDevice;
	class IRenderable;
	class RenderContext;

		class Mod_Renderer : public ::ayt::engine::modules::IModule
		{
		public:
			using WindowCloseCallback = std::function<void()>;
			virtual void setWindowCloseCallback(WindowCloseCallback onWindowClosed) = 0;
			virtual RenderDevice* getRenderDevice() = 0;
			virtual RenderContext& getRenderContext() = 0;

			virtual void registerRenderable(IRenderable* renderable) = 0;
			virtual void removeRenderable(IRenderable* renderable) = 0;
		};
}
