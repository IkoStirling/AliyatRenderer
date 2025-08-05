#pragma once
#include <memory>
#include <chrono>
#include "ECRegisterModule.h"
#include "Mod_EngineCore.h"

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
	void update();
	void close();

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
	void _regulateFrameRate(std::chrono::high_resolution_clock::time_point frameStartTime);

	void _updateFPSStats(int& frameCount, std::chrono::steady_clock::time_point& lastFpsUpdate);

private:
	bool _shouldClosed = false;

	bool _noLimitFPS = false;
	float _targetFPS = 60.f;          // 目标帧率
	float _invTargetFPS = 1.f / _targetFPS;			//帧率倒数，用作性能优化
	float _currentFPS = 0.0f;          // 实时帧率统计
	float _timeScale = 1.f;  // 时间缩放因子（0=暂停，0.5=慢放，2.0=加速）
	float _unscaledDeltaTime = 0.0f;  // 未缩放的真实帧时间

	std::atomic<float> _accumulatedTime{ 0.0f };  // 用于时间缩放累积
	std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
};

class AYEngineCoreAdapter : public Mod_EngineCore
{
public:
	void init() override {};

	void update(float delta_time) override {};

	void shutdown() override {};

	void setTargetFPS(float fps) override { AYEngineCore::getInstance().setTargetFPS(fps); }

	void setTimeScale(float scale) override { AYEngineCore::getInstance().setTimeScale(scale); }

	float getCurrentFPS() const override { return AYEngineCore::getInstance().getCurrentFPS(); }

	float getDeltaTime() const override { return AYEngineCore::getInstance().getDeltaTime(); }

	float getUnscaledDeltaTime() const override { return AYEngineCore::getInstance().getUnscaledDeltaTime(); }
};

REGISTER_MODULE_CLASS("EngineCore", AYEngineCoreAdapter)