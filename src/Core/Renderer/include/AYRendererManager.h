#pragma once
#include "Mod_Renderer.h"

class AYRenderDevice;
class AYRenderer;

class AYRendererManager : public Mod_Renderer
{
public:
	AYRendererManager() = default;
	void init() override;
	void update(float delta_time) override;

private:
	AYRenderDevice* _device = nullptr;      // OpenGL上下文管理
	AYRenderer* _renderer = nullptr;        // 具体绘制逻辑
	//AYResourceManager* resMgr;   // 资源管理接口
};

REGISTER_MODULE_CLASS("Renderer", AYRendererManager)