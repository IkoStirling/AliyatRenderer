#include "AYRendererManager.h"
#include "Mod_EngineCore.h"
#include "AYResourceManager.h"
#include "AYPath.h"

void AYRendererManager::init()
{
	_device = std::make_unique<AYRenderDevice>();
	if (!_device->init(1920, 1080))
		return;

	_renderer = std::make_unique<AYRenderer>(getRenderDevice());

	_animeMana = std::make_unique<AYAnimationManager>(getRenderDevice());

	_device->setViewportCallback([this](int width, int height) {
		getCameraSystem()->setViewportAll(glm::vec4(0, 0, width, height));
		});

	AYResourceManager::getInstance().loadAsync<AYTexture>(AYPath::Engine::getPresetTexturePath()+"Arrow.png",
		[](std::shared_ptr<AYTexture> texture) {
			//std::cout << "Texture loaded: " << texture->getWidth() << "x" << texture->getHeight() << std::endl;
		});

	_renderer->getMaterialManager()->createMaterial(
		{
			.baseColor = glm::vec4(.2f, 0.5f, 0.1f, 1.0f),
			.metallic = 1.f,
			.roughness = 0.3f,
			.type = STMaterial::Type::Opaque,
		}
		);
	_renderer->getMaterialManager()->createMaterial(
		{
			.baseColor = glm::vec4(.6f, 0.2f, 0.5f, 1.0f),
			.metallic = 0.3f,
			.roughness = 0.6f,
			.type = STMaterial::Type::Opaque,
		}
		);
	_renderer->getMaterialManager()->createMaterial(
		{
			.baseColor = glm::vec4(.1f, 0.7f, 0.2f, 1.0f),
			.metallic = 0.8f,
			.roughness = 0.1f,
			.type = STMaterial::Type::Opaque,
		}
		);

	auto lightManager = _renderer->getLightManager();
	lightManager->addDirectionalLight({
		.color = glm::vec3(1.0f, 0.95f, 0.9f),      // 暖白色
		.intensity = 1.2f,
		.direction = glm::vec3(-0.5f, -1.0f, -0.3f) // 左上后方照射
		});
	lightManager->addPointLight({
		.color = glm::vec3(0.8f, 0.9f, 1.0f),       // 冷色调
		.intensity = 0.8f,
		.position = glm::vec3(3.0f, 2.0f, 1.0f),
		.radius = 10.0f,
		.linear = 0.07f
		});
	lightManager->addSpotLight({
		.color = glm::vec3(1.0f, 0.9f, 0.7f),       // 暖黄色
		.intensity = 1.5f,
		.position = glm::vec3(0.0f, 3.0f, 2.0f),
		.direction = glm::vec3(0.0f, -0.7f, -0.5f), // 向下前方照射
		.cutOff = glm::cos(glm::radians(15.0f)),
		.outerCutOff = glm::cos(glm::radians(25.0f))
		});
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

	_updateCameraActive(delta_time);

	// 2. TODO: 执行实际渲染逻辑
	_renderAll(delta_time);

	// 3. 交换缓冲区
	glfwSwapBuffers(_device->getWindow());

	glfwPollEvents();
}

void AYRendererManager::shutdown()
{

}

void AYRendererManager::_renderAll(float delta_time)
{
	_renderer->clearScreen(_color.x, _color.y, _color.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//_renderer->getSkyboxRenderer()->render(_renderer->getRenderContext());
	_renderer->getCoreRenderer()->beginDraw();
	_displayDebugInfo();
	_renderer->getCoreRenderer()->endDraw();

	for (auto renderable : _renderables)
	{
		if (renderable)
			renderable->render(_renderer->getRenderContext());
	}


}

void AYRendererManager::_updateCameraActive(float delta_time)
{
	auto* camera = getCameraSystem()->getActiveCamera();
	auto& context = _renderer->getRenderContext();
	context.currentCamera = camera;
	context.validate();
	context.currentCamera->update(delta_time);
	const auto& viewport = context.currentCamera->getViewport();
	_device->getGLStateManager()->setViewport(viewport.x, viewport.y, viewport.z, viewport.w);

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
	_renderer->getFontRenderer()->renderText(fps, 25.0f, 25.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	auto* dr = _renderer->getCoreRenderer();

	/*dr->drawLine2D({ {glm::vec2(10.f),0}, glm::vec4(0, 1, 0, 1) },
		{ {glm::vec2(500.f),0}, glm::vec4(0, 0, 1, 1) },
		AYCoreRenderer::Space::World);
	dr->drawLine2D({ {glm::vec3(100.f, 100.f, 0.f)}, glm::vec4(0, 1, 0, 1) },
		{ {glm::vec3(600.f, 500.f, 0.f)}, glm::vec4(0, 1, 0, 1) },
		AYCoreRenderer::Space::World);
	dr->drawLine2D({ glm::vec3(300.f, 100.f, 0.f), glm::vec4(0, 1, 0, 1) },
		{ glm::vec3(-600.f, 500.f, 0.f),
		glm::vec4(0, 1, 0, 1) },
		AYCoreRenderer::Space::World);
	dr->drawArrow2D({}, glm::vec3(100.f, 400.f, 0.f), glm::vec3(600.f, 500.f, 0.f), 20.f, glm::vec4(1.f, 0.f, 0.f, 1.f));*/
	for (int i = 1; i < 100; i++) {
		dr->drawRect2D({ glm::vec3(i * 100.f, 0, 0) }, glm::vec2(50.f), 1, true);
		dr->drawRect2D({ glm::vec3(i * -100.f, 0, 0), glm::vec3(0, i * 10.f, 0) }, glm::vec2(30), 2, false);
	}
	for (int i = 1; i < 20; i++) {
		for (int j = 1; j < 20; j++) {
			dr->drawBox3D({ glm::vec3(i * -100.f + 1000.f, 0, j * -100.f + 1000.f) }, glm::vec3(20.f), 3, false, AYCoreRenderer::Space::World);
		}
	}

	dr->drawCircle2D({ glm::vec3(-50.f) }, 100.f, 2, 32, true, AYCoreRenderer::Space::World);
	dr->drawCircle2D({ glm::vec3( 50.f) }, 100.f, 2, 32, false, AYCoreRenderer::Space::World);
	

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
