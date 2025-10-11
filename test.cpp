#pragma once
#include <iostream>
#include "AYEngineCore.h"
#include "AYPhysicsSystem.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"
#include "AYECSEngine.h"
#include "AYAVEngine.h"
#include "AYVideo.h"
#include <spdlog/spdlog.h>
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <opencv2/core/utils/logger.hpp>

#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
#define ENABLE_CONSOLE_LOG 1
#else
#define ENABLE_CONSOLE_LOG 0
#endif

void setLocalOption()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
}

void setLogOption()
{
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %s:%# - %v");
	spdlog::set_level(spdlog::level::trace);
	auto persistentSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
		"./logs/persistent.log",
		1024 * 1024 * 10,
		3
	);
	auto runtimeSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
		"./logs/runtime.log",
		true
	);
	std::vector<spdlog::sink_ptr> runtime = { runtimeSink };
	std::vector<spdlog::sink_ptr> persistent = { persistentSink };

#if ENABLE_CONSOLE_LOG
	// 控制台 sink（仅 Debug 模式启用）：带颜色输出到终端
	runtime.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	persistent.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif

	auto persistentLogger = std::make_shared<spdlog::logger>("persistent", persistent.begin(), persistent.end());
	auto runtimeLogger = std::make_shared<spdlog::logger>("runtime", runtime.begin(), runtime.end());
	spdlog::register_logger(persistentLogger);
	spdlog::register_logger(runtimeLogger);
	spdlog::set_default_logger(runtimeLogger);

	persistentLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
	persistentLogger->set_level(spdlog::level::debug);
	runtimeLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
	runtimeLogger->set_level(spdlog::level::trace);

}

void avoidThirdPartyWarnning()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
	av_log_set_level(AV_LOG_ERROR);
}

int main()
{
	setLocalOption();
	setLogOption();
	avoidThirdPartyWarnning();

	auto persistentLogger = spdlog::get("persistent");
	auto runtimeLogger = spdlog::get("runtime");


	AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();
	return 0;
}

int WINAPI WinMain(
	HINSTANCE hInstance,      // 当前程序实例句柄
	HINSTANCE hPrevInstance,  // 以前实例句柄（已废弃，总是 NULL）
	LPSTR     lpCmdLine,      // 命令行参数（窄字符串，ANSI）
	int       nCmdShow        // 窗口显示方式（比如是否最小化等，GUI 有用）
) 
{
	return main();
}