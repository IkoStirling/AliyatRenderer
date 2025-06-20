#include "AYRendererManager.h"
#include "AYRenderer.h"
#include "Mod_EngineCore.h"
#include "AYResourceManager.h"
#include "2DRendering/AYAnimatedSprite.h"
#include "2DRendering/AYAnimationManager.h"


#include "Mod_InputSystem.h"

void AYRendererManager::init()
{
	_device = new AYRenderDevice();
	if (!_device->init(1920, 1080))
		return;
	_renderer = new AYRenderer(_device);

	tex_ID = loadTexture("assets/core/textures/checkerboard.png");

	_animaMana = new AYAnimationManager(_device);

	auto data = _animaMana->makeAnimationData(
		glm::vec2(100, 100),
		glm::vec2(800, 600),
		{
			{"idle01", 0, 6},
			{"walk01", 8, 8},
			{"atk01", 16, 6},
			{"atk02", 24, 6},
			{"eff01", 32, 4},
			{"dead01",40, 4}
		}
	);

	auto orcAtlas = _animaMana->loadAtlas(
		"orc",
		"assets/core/textures/sprite/testpack/Characters/Orc/Orc/Orc.png",
		glm::vec2(100, 100),
		data,
		{
			true,
			true,
			false,
			false,
			false,
			false
		});

	orcSprite = std::make_shared<AYAnimatedSprite>(
		_renderer->getSpriteRenderer(),
		orcAtlas
	);

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
	_renderer->clearScreen(0.0f, 0.0f, 0.0f, 1.0f);

	// 2. TODO: 执行实际渲染逻辑

	renderAll();

	_displayDebugInfo();
	// 3. 交换缓冲区
	glfwSwapBuffers(_device->getWindow());

	glfwPollEvents();
}

void AYRendererManager::renderAll()
{
	for (auto renderable : _renderables)
	{
		if (renderable)
			renderable->render(_context);
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

void AYRendererManager::setWindowCloseCallback(WindowCloseCallback onWindowClosed)
{
	_onWindowClosed = onWindowClosed;
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



	auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");

	auto speedX = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftX, 5.f });
	auto speedY = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftY, 5.f });

	static bool hDir = false;
	if (speedX < 0) hDir = true;
	else if (speedX > 0) hDir = false;
	else {}

	if (orcSprite->getController().isCurrentAnimationDone())
	{
		if (fabs(speedX) > fabs(speedY))
		{
			pos.x += speedX;
		}
		else
		{
			pos.y += speedY;
		}

		if (inputSystem->getUniversalInputState(MouseButtonInput{ GLFW_MOUSE_BUTTON_LEFT }) ||
			inputSystem->isActionActive("default.GamePad_X"))
		{
			orcSprite->playAnimation("atk01");
		}
		else if (speedX || speedY)
		{
			orcSprite->playAnimation("walk01");
		}
		else
		{
			orcSprite->playAnimation("idle01");
		}
	}
	
	orcSprite->update(delta);
	orcSprite->render(
		glm::vec2(-45 + pos.x, -55 + pos.y),
		glm::vec2(200, 200),
		0.f,
		glm::vec4(1.f),
		hDir,
		false,
		glm::vec2(0.5f));



	_renderer->getSpriteRenderer()->drawSprite(
		tex_ID,
		glm::vec2(x, y),  // 位置
		glm::vec2(300.0f, 300.0f),  // 大小
		0.0f,                       // 旋转
		glm::vec4(0.0f, 1.f, 0.f, 0.1f),// 颜色
		false,
		false,
		glm::vec2(0.5f, 0.5f)       // 原点(旋转中心)
	);
}
