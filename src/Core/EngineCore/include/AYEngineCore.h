#pragma once
#include <memory>
#include <chrono>
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
		帧率控制
	*/
	void _regulateFrameRate(std::chrono::high_resolution_clock::time_point frameStartTime);

	void _updateFPSStats(int& frameCount, std::chrono::steady_clock::time_point& lastFpsUpdate);

private:
	bool _shouldClosed = false;

	bool _noLimitFPS = false;
	float _targetFPS = 30.f;          // 目标帧率
	float _invTargetFPS = 1.f / _targetFPS;			//帧率倒数，用作性能优化
	float _currentFPS = 0.0f;          // 实时帧率统计
	float _timeScale = 0.02f;  // 时间缩放因子（0=暂停，0.5=慢放，2.0=加速）
	float _unscaledDeltaTime = 0.0f;  // 未缩放的真实帧时间

	std::atomic<float> _accumulatedTime{ 0.0f };  // 用于时间缩放累积
	std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
};