#pragma once
#include <memory>
#include <chrono>
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

public:
	void setTargetFPS(float fps);

	void setTimeScale(float scale);

	float getCurrentFPS() const;

	float getDeltaTime() const;

	float getUnscaledDeltaTime() const;

private:
	AYEngineCore() = default;
	~AYEngineCore();
	AYEngineCore(const AYEngineCore&) = delete;
	AYEngineCore(AYEngineCore&&) = delete;

private:
	/*
		֡�ʿ���
	*/
	void _regulateFrameRate(std::chrono::high_resolution_clock::time_point frameStartTime);

	void _updateFPSStats(int& frameCount, std::chrono::steady_clock::time_point& lastFpsUpdate);

private:
	bool _shouldClosed = false;

	bool _noLimitFPS = false;
	float _targetFPS = 30.f;          // Ŀ��֡��
	float _invTargetFPS = 1.f / _targetFPS;			//֡�ʵ��������������Ż�
	float _currentFPS = 0.0f;          // ʵʱ֡��ͳ��
	float _timeScale = 0.02f;  // ʱ���������ӣ�0=��ͣ��0.5=���ţ�2.0=���٣�
	float _unscaledDeltaTime = 0.0f;  // δ���ŵ���ʵ֡ʱ��

	std::atomic<float> _accumulatedTime{ 0.0f };  // ����ʱ�������ۻ�
	std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
};