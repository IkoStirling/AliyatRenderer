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
	AYRenderDevice* _device = nullptr;      // OpenGL�����Ĺ���
	AYRenderer* _renderer = nullptr;        // ��������߼�
	//AYResourceManager* resMgr;   // ��Դ����ӿ�
};

REGISTER_MODULE_CLASS("Renderer", AYRendererManager)