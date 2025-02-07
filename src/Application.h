#pragma once
#include <iostream>

#define app Application::getInstance()

class GLFWwindow;

using frameBufferResize = void(*)(int width, int height);
using keyHandle = void(*)(int key, int action, int mods);

class Application
{
public:
	~Application();
	
	bool init(const int& _width, const int& _height);
	bool update();
	void destory();

	uint32_t getWindowWidth() const;
	uint32_t getWindowHeight() const;

	static Application* getInstance();

	void setFrameBufferResizeCallBack(frameBufferResize _resizeFunc);
	void setKeyCallBack(keyHandle _keyFunc);
private:
	Application(const int& _width, const int& _height);
	Application() = delete;

	static void framebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void keyCallBack(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);

	static Application* appInstance;

private:
	uint32_t windowWidth;
	uint32_t windowHeight;
	GLFWwindow* windowInstance;
	frameBufferResize resizeFunc;
	keyHandle keyFunc;
};