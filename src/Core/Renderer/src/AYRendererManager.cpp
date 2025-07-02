#include "AYRendererManager.h"
#include "Mod_EngineCore.h"
#include "AYResourceManager.h"


void AYRendererManager::init()
{
	_device = std::make_unique<AYRenderDevice>();
	if (!_device->init(1920, 1080))
		return;
	_renderer = std::make_unique<AYRenderer>(getRenderDevice());
	_animeMana = std::make_unique<AYAnimationManager>(getRenderDevice());
	_cameraSystem = std::make_unique<AYCameraSystem>();





	tex_ID = loadTexture("assets/core/textures/500_497.png");
}

void AYRendererManager::update(float delta_time)
{
	if (glfwWindowShouldClose(_device->getWindow()))
	{
		glfwDestroyWindow(_device->getWindow());	// 销毁窗口和 OpenGL 上下文
		glfwTerminate();							// 终止 GLFW，释放所有资源
		_onWindowClosed();
		return;
	}
	delta = delta_time;
	_renderer->clearScreen(_color.x, _color.y, _color.z, 1.0f);
	_renderer->getCoreRenderer()->beginDraw();

	// 2. TODO: 执行实际渲染逻辑

	_renderAll(delta_time);

	_displayDebugInfo();

	_renderer->getCoreRenderer()->endDraw();
	// 3. 交换缓冲区
	glfwSwapBuffers(_device->getWindow());

	glfwPollEvents();
}

void AYRendererManager::_renderAll(float delta_time)
{
	_updateCameraActive(delta_time);

	for (auto renderable : _renderables)
	{
		if (renderable)
			renderable->render(_renderer->getRenderContext());
	}
}

void AYRendererManager::_updateCameraActive(float delta_time)
{
	auto* camera = _cameraSystem->getActiveCamera();
	auto& context = _renderer->getRenderContext();
	context.currentCamera = camera;
	context.validate();
	context.currentCamera->update(delta_time);
	const auto& viewport = context.currentCamera->getViewport();
	_renderer->setViewport(viewport.x, viewport.y, viewport.z, viewport.w);

	//.......
}

void AYRendererManager::registerRenderable(IAYRenderable* renderable)
{
	_renderables.push_back(renderable);
}

void AYRendererManager::removeRenderable(IAYRenderable* renderable)
{
	std::erase(_renderables, renderable);
}

void AYRendererManager::setWindowCloseCallback(WindowCloseCallback onWindowClosed)
{
	_onWindowClosed = onWindowClosed;
}

void AYRendererManager::setScreenCleanColor(const glm::vec3& color)
{
	_color = color;
}

AYRenderContext& AYRendererManager::getRenderContext()
{
	return _renderer->getRenderContext();
}

GLuint AYRendererManager::loadTexture(const std::string& path)
{
	auto tex = AYResourceManager::getInstance().load<AYTexture>(path);
	if (!tex || !tex->isLoaded()) {
		std::cerr << "Failed to load texture: " << path << std::endl;
		return 0;
	}
	return _device->createTexture2D(
		tex->getPixelData(),
		tex->getWidth(),
		tex->getHeight(),
		tex->getChannels());
}

AYAnimatedSprite* AYRendererManager::create2DSprite(std::shared_ptr<AYSpriteAtlas> atlas)
{
	return new AYAnimatedSprite(_renderer->getSpriteRenderer(), atlas);
}

void AYRendererManager::_displayDebugInfo()
{
	constexpr float speed = 200.f;
	static glm::vec2 pos = glm::vec2(200);
	static float x = 0.f, y = 0.f;
	static bool l = true, d = true;
	x += (l ? speed : -speed) * delta;
	y += (d ? speed : -speed) * delta;
	if (x > 1920.f)
		l = false;
	if (x < 0.f)
		l = true;
	if (y > 1080.f)
		d = false;
	if (y < 0.f)
		d = true;


	std::string fps = "当前fps: " + std::to_string(static_cast<int>(GetEngine()->getCurrentFPS()));
	_renderer->renderText(fps, 25.0f, 25.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	auto* dr = _renderer->getCoreRenderer();
	//dr->drawArrow2D({}, glm::vec3(100.f), glm::vec3(600.f), 20.f, glm::vec4(1.f, 0.f, 0.f, 1.f));
	//dr->drawLine2D(glm::vec2(0,0),glm::vec2(1920,1080),glm::vec4(1.f));

	dr->drawLine2D({ {glm::vec2(10.f),0}, glm::vec4(0, 1, 0, 1) },
		{ {glm::vec2(500.f),0}, glm::vec4(0, 0, 1, 1) },
		AYCoreRenderer::Space::World);

	dr->drawLine2D({ {glm::vec3(100.f, 100.f, 0.f)}, glm::vec4(0, 1, 0, 1) },
		{ {glm::vec3(600.f, 500.f, 0.f)}, glm::vec4(0, 1, 0, 1) },
		AYCoreRenderer::Space::World);

	dr->drawLine2D({ glm::vec3(300.f, 100.f, 0.f), glm::vec4(0, 1, 0, 1) },
		{ glm::vec3(-600.f, 500.f, 0.f),
		glm::vec4(0, 1, 0, 1) },
		AYCoreRenderer::Space::World);

	dr->drawArrow2D({}, glm::vec3(100.f, 400.f, 0.f), glm::vec3(600.f, -500.f, 0.f), 20.f, glm::vec4(1.f, 0.f, 0.f, 1.f));
	for (int i = 0; i < 100; i++) {
		dr->drawRect2D({ glm::vec3(i * 100.f, 0, 0) }, glm::vec2(50.f), glm::vec4(0, 1, 0, 1), false);
		dr->drawRect2D({ glm::vec3(i * -100.f, 0, 0) }, glm::vec2(30.f), glm::vec4(0, 0, 1, 1), true);
	}

	//_renderer->getSpriteRenderer()->drawSprite(
	//	tex_ID,
	//	//glm::vec2(x, y),  // 位置
	//	glm::vec2(0.f),
	//	glm::vec2(1200.0f),  // 大小
	//	0.0f,                       // 旋转
	//	glm::vec4(0.0f, 1.f, 0.f, 0.9f),// 颜色
	//	false,
	//	false,
	//	glm::vec2(0.5f, 0.5f)       // 原点(旋转中心)
	//);
}
