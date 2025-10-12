#pragma once
#include <iostream>
#include <sstream>
#include "core/core.h"
#include <mutex>
#include <functional>

#define app Application::getInstance()

class GLFWwindow;

class Application
{
public:
	~Application();

	bool init(const int& _width, const int& _height);
	bool update();
	void destory();

	uint32_t getWindowWidth() const;
	uint32_t getWindowHeight() const;
	GLFWwindow* getWindowInstance();

	static Application& getInstance();

	void setFrameBufferResizeCallBack(std::function<void(int width,int height)> _resizeFunc);
	void setKeyCallBack(std::function<void(int key, int action, int mods)> _keyFunc);
	void setMouseButtonCallback(std::function<void(int key, int action, int mods)> _mouseButtonFunc);
	void setCursorPosCallback(std::function<void(double xpos, double ypos)> _cursorPosFunc);
	void setScrollCallback(std::function<void(double xoffset, double yoffset)> _scrollFunc);
private:
	Application(const int& _width, const int& _height);
	Application() = delete;
	Application(const Application&) = delete;
	Application(Application&&) = delete;

	static void framebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void keyCallBack(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
	static void mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
	static void cursorPosCallback(GLFWwindow* _window, double _xpos, double _ypos);
	static void scrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset);

private:
	uint32_t windowWidth;
	uint32_t windowHeight;
	GLFWwindow* windowInstance;
	std::function<void(int width, int height)> resizeFunc;
	std::function<void(int key, int action, int mods)> keyFunc;
	std::function<void(int button, int action, int mods)> mouseButtonFunc;
	std::function<void(double xpos, double ypos)> cursorPosFunc;
	std::function<void(double xpos, double ypos)> scrollFunc;


};