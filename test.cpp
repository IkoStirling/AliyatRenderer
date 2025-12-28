#pragma once
#include <iostream>
#include "AYEngineCore.h"
#include <opencv2/core/utils/logger.hpp>

void setLocalOption()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
}

void avoidThirdPartyWarnning()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
	av_log_set_level(AV_LOG_ERROR);
}

int main()
{
	setLocalOption();
	avoidThirdPartyWarnning();

	ayt::engine::Core::getInstance().init();
	ayt::engine::Core::getInstance().start();
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