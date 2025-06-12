#include "AYRendererManager.h"
#include "AYRenderer.h"
#include "AYResourceManager.h"

void AYRendererManager::init()
{
	_device = new AYRenderDevice();
	if (!_device->init(1080, 720))
		return;
	_renderer = new AYRenderer(_device);
	
}

void AYRendererManager::update(float delta_time)
{
	if (glfwWindowShouldClose(_device->getWindow()))
		std::cout << "should close\n";
	_renderer->clearScreen(1.0f, 0.0f, 0.0f, 1.0f);

	// 2. TODO: Ö´ÐÐÊµ¼ÊäÖÈ¾Âß¼­

	// 3. ½»»»»º³åÇø
	glfwSwapBuffers(_device->getWindow());
	glfwWaitEventsTimeout(0.016);

	//glfwPollEvents();
}