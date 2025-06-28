#include "AYRendererManager.h"
#include "AYRendererManager.h"
#include "AYRendererManager.h"
#include "AYRenderer.h"
#include "Mod_EngineCore.h"
#include "AYResourceManager.h"



#include "Mod_InputSystem.h"

void AYRendererManager::init()
{
	_device = new AYRenderDevice();
	if (!_device->init(1920, 1080))
		return;
	_renderer = new AYRenderer(_device);
	_animeMana = new AYAnimationManager(_device);

	tex_ID = loadTexture("assets/core/textures/checkerboard.png");

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

void AYRendererManager::setScreenCleanColor(const glm::vec3& color)
{
	_color = color;
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
