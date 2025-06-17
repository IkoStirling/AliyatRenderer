#pragma once
#include "Mod_Renderer.h"

class AYRenderer;

class AYRendererManager : public Mod_Renderer
{

public:
	using GLuint = unsigned int;
public:
	AYRendererManager() = default;
	void init() override;
	void update(float delta_time) override;

	void setWindowCloseCallback(WindowCloseCallback onWindowClosed);
	AYRenderDevice* getRenderDevice() { return _device; };

	GLuint loadTexture(const std::string& path);

private:
	AYRenderDevice* _device = nullptr;      // OpenGL上下文管理
	AYRenderer* _renderer = nullptr;        // 具体绘制逻辑
	WindowCloseCallback _onWindowClosed;

private:
	//******************debug**********************
	void _displayDebugInfo();
	GLuint tex_ID = 0;
	float delta;
	class AYAnimatedSprite* _character;
	std::shared_ptr<AYAnimatedSprite> orcSprite;
	class AYAnimationManager* _animaMana;
};

REGISTER_MODULE_CLASS("Renderer", AYRendererManager)