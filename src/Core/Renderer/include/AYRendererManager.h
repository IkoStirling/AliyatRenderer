#pragma once
#include "Mod_Renderer.h"
#include "BaseRendering/IAYRenderable.h"
#include "2DRendering/AYAnimatedSprite.h"
#include "2DRendering/AYAnimationManager.h"
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

	void setWindowCloseCallback(WindowCloseCallback onWindowClosed);
	void setScreenCleanColor(const glm::vec3& color);

	AYRenderContext& getRenderContext() { return _context; }
	AYRenderDevice* getRenderDevice() { return _device; };
	AYAnimationManager* get2DAnimationManager() { return _animeMana; }
	GLuint loadTexture(const std::string& path);

	AYAnimatedSprite* create2DSprite(std::shared_ptr<AYSpriteAtlas> atlas);
private:
	AYRenderDevice* _device = nullptr;      // OpenGL�����Ĺ���
	AYRenderer* _renderer = nullptr;        // ��������߼�
	AYAnimationManager* _animeMana = nullptr;
	WindowCloseCallback _onWindowClosed;
	std::vector<IAYRenderable*> _renderables; //���������Ⱦ����
	AYRenderContext _context;

	glm::vec3 _color = { 0.f,0.f,0.f };
private:
	//******************debug**********************
	void _displayDebugInfo();
	GLuint tex_ID = 0;
	float delta;
	class AYAnimatedSprite* _character;
	std::shared_ptr<AYAnimatedSprite> orcSprite;
};

REGISTER_MODULE_CLASS("Renderer", AYRendererManager)