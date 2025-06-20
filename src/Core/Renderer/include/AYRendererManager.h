#pragma once
#include "Mod_Renderer.h"
#include "BaseRendering/IAYRenderable.h"
#include <vector>
class AYRenderer;

class AYRendererManager : public Mod_Renderer
{

public:
	using GLuint = unsigned int;
public:
	AYRendererManager() = default;
	void init() override;
	void update(float delta_time) override;
	void renderAll() override;
	void registerRenderable(IAYRenderable* renderable) override;
	void removeRenderable(IAYRenderable* renderable) override;
	AYRenderContext& getRenderContext() { return _context; }

	void setWindowCloseCallback(WindowCloseCallback onWindowClosed);

	AYRenderDevice* getRenderDevice() { return _device; };

	GLuint loadTexture(const std::string& path);

private:
	AYRenderDevice* _device = nullptr;      // OpenGL上下文管理
	AYRenderer* _renderer = nullptr;        // 具体绘制逻辑
	WindowCloseCallback _onWindowClosed;
	std::vector<IAYRenderable*> _renderables; //不管理可渲染对象
	AYRenderContext _context;

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