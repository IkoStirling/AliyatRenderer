#include "AYRendererManager.h"
#include "Mod_EngineCore.h"
#include "AYResourceManager.h"
#include "AYPath.h"
#include "AYSoundEngine.h"

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
	//AYResourceManager::getInstance().loadAsync<AYVideo>(AYPath::Engine::getPresetTexturePath()+"Arrow.png",
	//	[this](std::shared_ptr<AYVideo> video) {
	//		videos = video;
	//		videot = _device->createVideoTexture(video->getWidth(), video->getHeight());
	//	});
	//AYResourceManager::getInstance().loadAsync<AYVideo>("@videos/test_video.mp4",
	//	[this](std::shared_ptr<AYVideo> video) {
	//		videos = video;
	//	});

	AYResourceManager::getInstance().loadAsync<AYModel>("@models/sour-Miku-Creamy/sour.fbx",
		[this](std::shared_ptr<AYModel> model) {
			modelPmx = model;
		});

	//AYResourceManager::getInstance().loadAsync<AYVideo>("@videos/bad_apple.mp4",
	//	[this](std::shared_ptr<AYVideo> video) {
	//		video->play();
	//		videos = video;
	//	});

	//videos = AYResourceManager::getInstance().load<AYVideo>("@videos/test_video.mp4");
	
	auto model2 = AYResourceManager::getInstance().load<AYModel>("@models/cube.fbx");
	tex_ID = loadTexture("@textures/500_497.png");
	tex_ID2 = loadTexture("@textures/1918_1100.png");
	tex_ID2 = loadTexture("@textures/checkerboard.png");

	_renderer->getMaterialManager()->createMaterial(
		{
			.name = "1",
			.baseColor = glm::vec4(.2f, 0.5f, 0.1f, 1.0f),
			.metallic = 1.f,
			.roughness = 0.3f,
			.type = STMaterial::Type::Opaque,
		}
		);
	_renderer->getMaterialManager()->createMaterial(
		{
			.name = "2",
			.baseColor = glm::vec4(.6f, 0.2f, 0.5f, 1.0f),
			.metallic = 0.3f,
			.roughness = 0.6f,
			.type = STMaterial::Type::Opaque,
		}
		);
	_renderer->getMaterialManager()->createMaterial(
		{
			.name = "3",
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

	auto cameraSystem = _renderer->getCameraSystem();
	auto camera = cameraSystem->getActiveCamera();
	auto ppm = camera->getPixelPerMeter();

	std::string str = "测试测字串012%￥@#@\n测试测字串012%￥@#@";
	auto id = _renderer->getUIRenderer()->createRectangle(glm::vec3(1,1,0)* ppm, glm::vec3(3,1,0) * ppm, glm::vec4(1));
	_renderer->getUIRenderer()->setText(id,"button");
	_renderer->getUIRenderer()->setOnClicked(id, []() {
		std::cout << "clicked!\n";
		});
	_renderer->getUIRenderer()->setOnUnhovered(id, [id, ui = _renderer->getUIRenderer()]() {
		std::cout << "setOnUnhovered!\n";
		ui->setColor(id, glm::vec4(1, 1, 1, 1));
		});
	_renderer->getUIRenderer()->setOnHovered(id, [id, ui = _renderer->getUIRenderer()]() {
		std::cout << "OnHovered!\n";
		ui->setColor(id, glm::vec4(0, 0, 0, 1));
		});
	_renderer->getUIRenderer()->createText(str, glm::vec3(1, 1, 0)* ppm, glm::vec4(1, 1, 1, 1), 1.f);
	//_renderer->getUIRenderer()->setOnUnhovered(id, [&]() {
	//	_renderer->getUIRenderer()->setColor(id, glm::vec4(0, 0, 0, 1));
	//	});
	//_renderer->getUIRenderer()->createRectangle(glm::vec3(2,2,0)* ppm, glm::vec3(4) * ppm, glm::vec4(1));
	//_renderer->getUIRenderer()->createRectangle(glm::vec3(10,10,0)* ppm, glm::vec3(4) * ppm, glm::vec4(1));
	//_renderer->getUIRenderer()->createRectangle(glm::vec3(10,10,0)* ppm, glm::vec3(4) * ppm, glm::vec4(1));
	//_renderer->getUIRenderer()->createRectangle(glm::vec3(10,10,0)* ppm, glm::vec3(4) * ppm, glm::vec4(1));
	//_renderer->getUIRenderer()->createRectangle(glm::vec3(10,10,0)* ppm, glm::vec3(4) * ppm, glm::vec4(1));
	
	_renderer->getUIRenderer()->createText(str, glm::vec3(2, 11, 0)* ppm, glm::vec4(1,1,1,1), 1.f);
	_renderer->getUIRenderer()->createText(str, glm::vec3(2, 12, 0)* ppm, glm::vec4(1,1,1,1), 1.f);
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
	_animeMana->shutdown();
	_renderer->shutdown();
	_device->shutdown();
}

void AYRendererManager::_renderAll(float delta_time)
{
	_renderer->clearScreen(_color.x, _color.y, _color.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_renderer->getSkyboxRenderer()->render(_renderer->getRenderContext());

	for (auto renderable : _renderables)
	{
		if (renderable)
			renderable->render(_renderer->getRenderContext());
	}

	_renderer->getCoreRenderer()->beginDraw();
	_debugDraw(false);
	_displayDebugInfo();
	_renderer->getCoreRenderer()->endDraw();

	_renderer->getUIRenderer()->beginUIFrame();
	_debugDraw(true);
	_renderer->getUIRenderer()->renderUI();
	_renderer->getUIRenderer()->endUIFrame();
}

void AYRendererManager::_updateCameraActive(float delta_time)
{
	auto* cameraSystem = getCameraSystem();
	auto* camera = cameraSystem->getActiveCamera();
	auto cameraID = cameraSystem->getActiveCameraID();

	auto& context = _renderer->getRenderContext();
	context.currentCameraID = cameraID;
	context.validate();

	cameraSystem->update(delta_time);

	const auto& viewport = camera->getViewport();
	_device->getGLStateManager()->setViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}

void AYRendererManager::_debugDraw(bool isUI)
{
	if (isUI)
	{
		for (auto& [_, debugs] : _debugDrawsScreen)
		{
			debugs(getRenderer(), getRenderDevice());
		}
	}
	else
	{
		for (auto& [_, debugs] : _debugDrawsWorld)
		{
			debugs(getRenderer(), getRenderDevice());
		}
	}

}

void AYRendererManager::registerRenderable(IAYRenderable* renderable)
{
	_renderables.push_back(renderable);
}

void AYRendererManager::removeRenderable(IAYRenderable* renderable)
{
	std::erase(_renderables, renderable);
}

int AYRendererManager::addDebugDraw(bool isUI, DebugDrawFunc callback)
{
	int id;
	if (!_freeDebugDraws.empty())
	{
		id = _freeDebugDraws.back();
		_freeDebugDraws.pop_back();
	}
	else
	{
		id = ++_debugDrawCount;
	}
	if(isUI)
		_debugDrawsScreen.try_emplace(id, callback);
	else
		_debugDrawsWorld.try_emplace(id, callback);
	return id;
}

void AYRendererManager::removeDebugDraw(int callbackId)
{
	if (auto it = _debugDrawsScreen.find(callbackId); it != _debugDrawsScreen.end())
	{
		_debugDrawsScreen.erase(it);
		_freeDebugDraws.push_back(callbackId);
	}
	else if (auto it = _debugDrawsWorld.find(callbackId); it != _debugDrawsWorld.end())
	{
		_debugDrawsWorld.erase(it);
		_freeDebugDraws.push_back(callbackId);
	}
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

	auto& context = getRenderContext();
	auto cameraSystem = _renderer->getCameraSystem();
	auto camera = cameraSystem->getCamera(context.currentCameraID);

	auto ppm = camera->getPixelPerMeter();
	auto* dr = _renderer->getCoreRenderer();

	auto mt = _renderer->getMaterialManager();
	auto mat1 = mt->getMaterial("1").id;
	auto mat2 = mt->getMaterial("2").id;
	auto mat3 = mt->getMaterial("3").id;
	for (int i = 1; i < 100; i++) {
		dr->drawRect2D({ glm::vec3(i * 10.f, 0, 0) }, glm::vec2(1.f), mat1, true);
		dr->drawRect2D({ glm::vec3(i * -10.f, 0, 0), glm::vec3(0, i * 10.f, 0) }, glm::vec2(0.5f), mat2, false);
	}
	for (int i = 1; i < 20; i++) {
		for (int j = 1; j < 20; j++) {
			dr->drawBox3D({ glm::vec3(i * -10.f + 10.f, 0, j * -10.f + 10.f)}, glm::vec3(0.5f), mat3, false, AYCoreRenderer::Space::World);
		}
	}

	dr->drawArrow2D({}, glm::vec3(0), glm::vec3(2,2,0)*ppm, 0.3f * ppm, glm::vec4(1), AYCoreRenderer::Space::Screen);
	//dr->drawLine2D(glm::vec2(0), glm::vec2(5,5), glm::vec4(1));

	// 死区框
	//dr->drawRect2D({ glm::vec3(1920 * 0.5f, 1080 * 0.5f, 0)}, glm::vec2(1920 *0.4f, 1080 * 0.4f), mat2, true, AYCoreRenderer::Space::Screen);
	
	//_renderer->getSpriteRenderer()->drawSprite(
	//	tex_ID,
	//	{ glm::vec3(0, 0, -0.9f) },
	//	//{},
	//	glm::vec2(0),
	//	glm::vec2(1),
	//	glm::vec2(10.0f),  // 大小
	//	glm::vec4(1.0f, 1.f, 1.f, 1.f),// 颜色
	//	false,
	//	false,
	//	glm::vec2(0.5f, 0.5f)       // 原点(旋转中心)
	//);

	auto model = AYResourceManager::getInstance().load<AYModel>("@models/suzanne.fbx");
	for (int i = 1; i < 100; i++) {
		_renderer->getCoreRenderer()->drawMesh(
			{
				glm::vec3(i * -5 + 250, 5, 5),
				glm::vec3(glm::radians(-90.f),0,0)
			},
			model->getMeshes()[0],
			false,
			AYCoreRenderer::Space::World
		);
	}


	if (modelPmx && 0)
	{
		auto& meshes = modelPmx->getMeshes();
		for (int i = 0; i < meshes.size(); i++) {
			_renderer->getCoreRenderer()->drawMesh(
				{
					glm::vec3(0, 0, 4),
					glm::vec3(glm::radians(-90.f),0,0)
				},
				meshes[i],
				false,
				AYCoreRenderer::Space::World
			);
		}
	}



	if (0)
	{
		static bool t = true;
		if (t)
		{
			t = false;
			auto soundEngine = GET_CAST_MODULE(AYSoundEngine, "SoundEngine");
			videos = soundEngine->playVideo("@videos/bad_apple.mp4");
			videot = _device->createTexture2D(nullptr, videos->getWidth(), videos->getHeight());
		}
		//"@videos/bad_apple.mp4"

		if (videos)
		{
			const uint8_t* pixelData = videos->getCurrentFramePixelData();
			_device->updateTexture(videot, pixelData,
				videos->getWidth(), videos->getHeight());
			_renderer->getSpriteRenderer()->drawSprite(
				videot,
				{ glm::vec3(0,0,-1) },
				glm::vec2(0),
				glm::vec2(1),
				glm::vec2(videos->getWidth(), videos->getHeight()) / ppm,  // 大小
				glm::vec4(1.0f, 1.f, 1.f, 0.9f),// 颜色
				false,
				false,
				glm::vec2(0.5f, 0.5f)       // 原点(旋转中心)
			);
		}
	}

	/*dr->drawCircle2D({ glm::vec3(50.f) }, 100.f, 2, 32, false, AYCoreRenderer::Space::World);
	dr->drawCircle2D({ glm::vec3(-50.f) }, 100.f, 2, 32, true, AYCoreRenderer::Space::World);*/
}
