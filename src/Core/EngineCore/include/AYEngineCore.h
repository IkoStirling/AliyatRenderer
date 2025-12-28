#pragma once
#include <memory>
#include <chrono>
#include "ECRegisterModule.h"
#include "Mod_EngineCore.h"

namespace ayt::engine
{
	class EventSystem;

	class Core
	{
	public:
		static Core& getInstance();

		void init();
		void start();
		void update();
		void close();

	public:
		void setLogger(std::unique_ptr<log::Logger>&& logger);

	private:
		std::unique_ptr<log::Logger> _logger;

	public:
		void setTargetFPS(float fps);

		void setTimeScale(float scale);

		float getCurrentFPS() const;

		float getDeltaTime() const;

		float getUnscaledDeltaTime() const;

	private:
		Core() = default;
		~Core();
		Core(const Core&) = delete;
		Core(Core&&) = delete;

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

	class CoreAdapter : public Mod_EngineCore
	{
	public:
		void init() override {};

		void update(float delta_time) override {};

		void shutdown() override {};

		void setTargetFPS(float fps) override { Core::getInstance().setTargetFPS(fps); }

		void setTimeScale(float scale) override { Core::getInstance().setTimeScale(scale); }

		float getCurrentFPS() const override { return Core::getInstance().getCurrentFPS(); }

		float getDeltaTime() const override { return Core::getInstance().getDeltaTime(); }

		float getUnscaledDeltaTime() const override { return Core::getInstance().getUnscaledDeltaTime(); }
	};

	REGISTER_MODULE_CLASS("EngineCore", CoreAdapter)
}
