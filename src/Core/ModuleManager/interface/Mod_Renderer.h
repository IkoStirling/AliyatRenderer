#pragma once
#include "IAYModule.h"
#include <functional>

class AYRenderDevice;

class Mod_Renderer : public IAYModule 
{
public:
	using WindowCloseCallback = std::function<void()>;
	virtual void setWindowCloseCallback(WindowCloseCallback onWindowClosed) = 0;
	virtual AYRenderDevice* getRenderDevice() = 0;
};