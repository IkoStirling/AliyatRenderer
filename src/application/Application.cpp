#include "Application.h"

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
	if (self->resizeFunc)
		self->resizeFunc(_width, _height);
}

void Application::keyCallBack(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	Application* self = (Application*)glfwGetWindowUserPointer(_window);
	if (self->keyFunc)
		self->keyFunc(_key, _action, _mods);
}

void Application::mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods)
{
	Application* self = (Application*)glfwGetWindowUserPointer(_window);
	if (self->mouseButtonFunc)
		self->mouseButtonFunc(_button, _action, _mods);
}

void Application::cursorPosCallback(GLFWwindow* _window, double _xpos, double _ypos)
{
	Application* self = (Application*)glfwGetWindowUserPointer(_window);
	if (self->cursorPosFunc)
		self->cursorPosFunc(_xpos, _ypos);
}

void Application::scrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset)
{
	Application* self = (Application*)glfwGetWindowUserPointer(_window);
	if (self->scrollFunc)
		self->scrollFunc(_xoffset, _yoffset);
}

Application& Application::getInstance()
{
	static Application appInstance(800, 600);
	return appInstance;
}

void Application::setFrameBufferResizeCallBack(std::function<void(int width, int height)> _resizeFunc)
{
	resizeFunc = _resizeFunc;
}

void Application::setKeyCallBack(std::function<void(int key, int action, int mods)> _keyFunc)
{
	keyFunc = _keyFunc;
}

void Application::setMouseButtonCallback(std::function<void(int key, int action, int mods)> _mouseButtonFunc)
{
	mouseButtonFunc = _mouseButtonFunc;
}

void Application::setCursorPosCallback(std::function<void(double xpos, double ypos)> _cursorPosFunc)
{
	cursorPosFunc = _cursorPosFunc;
}

void Application::setScrollCallback(std::function<void(double xoffset, double yoffset)> _scrollFunc)
{
	scrollFunc = _scrollFunc;
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
	glfwSetMouseButtonCallback(windowInstance, mouseButtonCallback);
	glfwSetCursorPosCallback(windowInstance, cursorPosCallback);
	glfwSetScrollCallback(windowInstance, scrollCallback);

	glfwSetWindowUserPointer(windowInstance, this);

	glfwSwapInterval(1);
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

GLFWwindow* Application::getWindowInstance()
{
	return windowInstance;
}
