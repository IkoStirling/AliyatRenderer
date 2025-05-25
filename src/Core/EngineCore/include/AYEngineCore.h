#pragma once
#include <memory>
#include "ECRegisterModule.h"


class AYEventSystem;

/*
	��������ʲô��
	�����ں����룿
	        X
	��������Ⱦģ����д�������������ϵͳ���д���

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