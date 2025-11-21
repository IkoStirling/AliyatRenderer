#pragma once
#include "Mod_Renderer.h"
#include "AYRenderDevice.h"
#include "AYRenderer.h"
#include "AYModel.h"
#include "BaseRendering/IAYRenderable.h"
#include "2DRendering/AYAnimatedSprite.h"
#include "2DRendering/AYAnimationManager.h"
#include <vector>
#include <AYVideo.h>

/*
	后期优化策略：深度预缓冲-》延迟渲染，前提：完善渲染管线
*/
class AYRendererManager : public Mod_Renderer
{

public:
	using GLuint = unsigned int;
	using DebugDrawFunc = std::function<void(AYRenderer*, AYRenderDevice*)>;
public:
	AYRendererManager() = default;
	void init() override;
	void shutdown() override;
	void update(float delta_time) override;
	void registerRenderable(IAYRenderable* renderable) override;
	void removeRenderable(IAYRenderable* renderable) override;
	int addDebugDraw(bool isUI, DebugDrawFunc callback);
	void removeDebugDraw(int callbackId);

	void setWindowCloseCallback(WindowCloseCallback onWindowClosed);
	void setScreenCleanColor(const AYMath::Vector3& color);
	void switchRenderModle(bool isBgfx);

	AYRenderContext& getRenderContext();
	AYRenderDevice* getRenderDevice() { return _device.get(); };
	AYRenderer* getRenderer() { return _renderer.get(); };
	AYAnimationManager* get2DAnimationManager() { return _animeMana.get(); }
	AYCameraSystem* getCameraSystem() { return _renderer->getCameraSystem(); }
	
	GLuint loadTexture(const std::string& path);

	AYAnimatedSprite* create2DSprite(std::shared_ptr<AYSpriteAtlas> atlas);
private:
	void _renderAll(float delta_time);
	void _renderAllB(float delta_time);
	void _updateCameraActive(float delta_time);

private:
	void _debugDraw(bool isUI);
	bool _enableDebugDraws = true;
	std::unordered_map<int, DebugDrawFunc> _debugDrawsWorld;
	std::unordered_map<int, DebugDrawFunc> _debugDrawsScreen;
	std::vector<int> _freeDebugDraws;
	int _debugDrawCount = 0;
private:
	bool _useBgfx = false;
	std::unique_ptr<AYRenderDevice> _device = nullptr;      // OpenGL上下文管理
	std::unique_ptr<AYRenderer> _renderer = nullptr;        // 具体绘制逻辑
	std::unique_ptr<AYAnimationManager> _animeMana = nullptr;

	WindowCloseCallback _onWindowClosed;
	std::vector<IAYRenderable*> _renderables; //不管理可渲染对象

	//AYMath::Vector3 _color = { 0.f,0.f,0.f };
	AYMath::Vector3 _color = { 0.2f,0.3f,0.3f };
private:
	//******************debug**********************
	void _displayDebugInfo();
	GLuint tex_ID = 0;
	GLuint tex_ID2 = 0;
	float delta;
	class AYAnimatedSprite* _character;
	std::shared_ptr<AYAnimatedSprite> orcSprite;
	std::shared_ptr<AYVideo> videos;
	std::shared_ptr<AYModel> modelPmx;
};

REGISTER_MODULE_CLASS("Renderer", AYRendererManager)