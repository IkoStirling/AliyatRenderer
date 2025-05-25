#pragma once
#include <memory>
#include "ECRegisterModule.h"


class AYEventSystem;

/*
	这个类管理什么？
	管理窗口和输入？
	        X
	窗口由渲染模块进行处理，输入由输入系统进行处理

*/
class AYEngineCore
{
private:

public:

public:
	static AYEngineCore& getInstance();

	void init();
	void start();
	bool update();

private:
	AYEngineCore() = default;
	~AYEngineCore();
	AYEngineCore(const AYEngineCore&) = delete;
	AYEngineCore(AYEngineCore&&) = delete;

private:
	bool _shouldClosed{ false };
};