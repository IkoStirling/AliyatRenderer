#pragma once
#include "Mod_Renderer.h"
#include "AYRenderDevice.h"
#include "AYRenderer.h"
#include "BaseRendering/IAYRenderable.h"
#include "BaseRendering/Camera/AYCameraSystem.h"
#include "2DRendering/AYAnimatedSprite.h"
#include "2DRendering/AYAnimationManager.h"
#include <vector>

class AYRendererManager : public Mod_Renderer
{

public:
	using GLuint = unsigned int;
public:
	AYRendererManager() = default;
	void init() override;
	void update(float delta_time) override;
	void registerRenderable(IAYRenderable* renderable) override;
	void removeRenderable(IAYRenderable* renderable) override;

	void setWindowCloseCallback(WindowCloseCallback onWindowClosed);
	void setScreenCleanColor(const glm::vec3& color);

	AYRenderContext& getRenderContext();
	AYRenderDevice* getRenderDevice() { return _device.get(); };
	AYAnimationManager* get2DAnimationManager() { return _animeMana.get(); }
	AYCameraSystem* getCameraSystem() { return _cameraSystem.get(); }
	GLuint loadTexture(const std::string& path);

	AYAnimatedSprite* create2DSprite(std::shared_ptr<AYSpriteAtlas> atlas);
private:
	void _renderAll(float delta_time);
	void _updateCameraActive(float delta_time);

private:
	std::unique_ptr<AYRenderDevice> _device = nullptr;      // OpenGL上下文管理
	std::unique_ptr<AYRenderer> _renderer = nullptr;        // 具体绘制逻辑
	std::unique_ptr<AYAnimationManager> _animeMana = nullptr;
	std::unique_ptr<AYCameraSystem> _cameraSystem = nullptr;

	WindowCloseCallback _onWindowClosed;
	std::vector<IAYRenderable*> _renderables; //不管理可渲染对象

	glm::vec3 _color = { 0.2f,0.3f,0.3f };
private:
	//******************debug**********************
	void _displayDebugInfo();
	GLuint tex_ID = 0;
	float delta;
	class AYAnimatedSprite* _character;
	std::shared_ptr<AYAnimatedSprite> orcSprite;
};

REGISTER_MODULE_CLASS("Renderer", AYRendererManager)