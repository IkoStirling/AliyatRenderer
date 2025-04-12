#pragma once
#include <memory>

#define GetEngine() AYEngineCore::getInstance()
#define GetEventSystem() AYEngineCore::getInstance().getEventSystem()

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
	std::shared_ptr<AYEventSystem> _eventSystem;
public:
	std::shared_ptr<AYEventSystem> getEventSystem();
public:
	static AYEngineCore& getInstance();

	void init();
	void start() {};
	bool update() {};

private:
	AYEngineCore();
	~AYEngineCore();
	AYEngineCore(const AYEngineCore&) = delete;
	AYEngineCore(AYEngineCore&&) = delete;

private:
	bool _shouldClosed{ false };
};