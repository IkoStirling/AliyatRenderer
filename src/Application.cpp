#include "Application.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "checkError.h"


Application* Application::appInstance = nullptr;

Application::Application(const int& _width, const int& _height) :
	windowWidth(_width),
	windowHeight(_height),
	windowInstance(nullptr),
	resizeFunc(nullptr),
	keyFunc(nullptr)
{

}

void Application::framebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	Application* self = (Application*)glfwGetWindowUserPointer(_window);
	if(self->resizeFunc)
		self->resizeFunc(_width, _height);
}

void Application::keyCallBack(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	Application* self = (Application*)glfwGetWindowUserPointer(_window);
	if (self->keyFunc)
		self->keyFunc(_key, _action,_mods);
}

Application* Application::getInstance()
{
	if (!appInstance)
	{
		appInstance = new Application(800, 600);
	}
	return appInstance;
}

void Application::setFrameBufferResizeCallBack(frameBufferResize _resizeFunc)
{
	resizeFunc = _resizeFunc;
}

void Application::setKeyCallBack(keyHandle _keyFunc)
{
	keyFunc = _keyFunc;
}

Application::~Application()
{

}
bool Application::init(const int& _width, const int& _height)
{
	windowWidth = _width;
	windowHeight = _height;
	if (!glfwInit())	return false;

	/*
	* version 4.6, core profile
	*/
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	windowInstance = glfwCreateWindow(windowWidth, windowHeight, "AlitatRenderer", NULL, NULL);
	if (!windowInstance) return false;
	glfwMakeContextCurrent(windowInstance);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;
	
	glfwSetFramebufferSizeCallback(windowInstance, framebufferSizeCallback);
	glfwSetKeyCallback(windowInstance, keyCallBack);

	glfwSetWindowUserPointer(windowInstance, this);
	return true;
}
bool Application::update()
{
	if (glfwWindowShouldClose(windowInstance)) return false;

	glfwPollEvents();
	glfwSwapBuffers(windowInstance);
	return true;
}
void Application::destory()
{
	glfwDestroyWindow(windowInstance);
	glfwTerminate();
}
uint32_t Application::getWindowWidth() const
{
	return windowWidth;
}
uint32_t Application::getWindowHeight() const
{
	return windowHeight;
}
