#pragma once
#include "IAYModule.h"
#include <functional>

class AYRenderDevice;
class IAYRenderable;
class AYRenderContext;

class Mod_Renderer : public IAYModule 
{
public:
	using WindowCloseCallback = std::function<void()>;
	virtual void setWindowCloseCallback(WindowCloseCallback onWindowClosed) = 0;
	virtual AYRenderDevice* getRenderDevice() = 0;
	virtual AYRenderContext& getRenderContext() = 0;

	virtual void registerRenderable(IAYRenderable* renderable) = 0;
	virtual void removeRenderable(IAYRenderable* renderable) = 0;
};