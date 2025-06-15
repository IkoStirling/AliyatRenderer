#include "AYRendererManager.h"
#include "AYRenderer.h"
#include "Mod_EngineCore.h"
#include "AYResourceManager.h"
#include "AYAnimatedSprite.h"

void AYRendererManager::init()
{
	_device = new AYRenderDevice();
	if (!_device->init(1920, 1080))
		return;
	_renderer = new AYRenderer(_device);
	
	tex_ID = loadTexture("assets/core/textures/checkerboard.png");

	auto spriteAtlas = loadTexture("assets/core/textures/sprite/testpack/Characters/Orc/Orc/Orc-Attack01.png");
	_character = new AYAnimatedSprite(_renderer->getSpriteRenderer(), spriteAtlas);
	glm::vec2 spriteSize(100, 100);
	glm::vec2 atlasSize(600, 100); // 整个图集尺寸
	AYSpriteAnimator::Animation attackAnim;
	for (int i = 0; i < 6; i++) {
		AYSpriteAnimator::AnimationFrame frame;
		frame.uvOffset = glm::vec2(i * 100.0f / 600.0f, 0.0f); // 第一行走动画
		frame.uvSize = glm::vec2(100.0f / 600.0f, 100.0f / 100.0f);
		frame.duration = 0.1f;
		attackAnim.frames.push_back(frame);
	}
	_character->getAnimator().addAnimation("atk", attackAnim.frames, true);
	_character->getAnimator().play("atk");
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


	_displayDebugInfo();
	// 3. 交换缓冲区
	glfwSwapBuffers(_device->getWindow());

	glfwPollEvents();
}

void AYRendererManager::setWindowCloseCallback(WindowCloseCallback onWindowClosed)
{
	_onWindowClosed = onWindowClosed;
}

GLuint AYRendererManager::loadTexture(const std::string& path)
{
	auto tex = AYResourceManager::getInstance().load<AYTexture>(path);
	auto id = _device->createTexture2D(tex->getPixelData(), tex->getWidth(), tex->getHeight(), tex->getChannels());
	//AYResourceManager::getInstance().unloadResource(path);
	return id;
}

void AYRendererManager::_displayDebugInfo()
{
	std::string fps = "侍fps: " + std::to_string(static_cast<int>(GetEngine()->getCurrentFPS()));
	_renderer->renderText(fps, 25.0f, 25.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	constexpr float speed = 200.f;
	static float x = 0.f, y = 0.f;
	static bool l = true, d = true;
	x += (l? speed : -speed) * delta;
	y += (d? speed : -speed) * delta;
	if (x > 1920.f)
		l = false;
	if (x < 0.f)
		l = true;
	if (y > 1080.f)
		d = false;
	if (y < 0.f)
		d = true;

	_renderer->getSpriteRenderer()->drawSprite(
		tex_ID,
		glm::vec2(x, y),  // 位置
		glm::vec2(300.0f, 300.0f),  // 大小
		0.0f,                       // 旋转
		glm::vec4(0.0f,1.f,0.f,1.f),// 颜色
		glm::vec2(0.5f, 0.5f)       // 原点(旋转中心)
	);

	_character->update(delta);
	_character->render(glm::vec2(-45, -55+1080), glm::vec2(100, 100));
	
}
