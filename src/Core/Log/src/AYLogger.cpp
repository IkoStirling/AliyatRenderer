#include "AYLogger.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

ayt::engine::log::Logger::Logger()
{
}

ayt::engine::log::Logger::~Logger()
{
}

void ayt::engine::log::Logger::init()
{
	setConsoleLog(true);
	flush();
}

void ayt::engine::log::Logger::flush()
{
	spdlog::drop_all();
	_createLogger();
}

void ayt::engine::log::Logger::close()
{
	spdlog::drop_all();
}

void ayt::engine::log::Logger::setConsoleLog(bool enable)
{
	_enableConsoleLog.store(enable);
}

void ayt::engine::log::Logger::setPattern(const std::string& pattern)
{
	_getPersistentLogger()->set_pattern(pattern);
	_getRuntimeLogger()->set_pattern(pattern);
}

void ayt::engine::log::Logger::setLevel(LogLevel level)
{
	_getPersistentLogger()->set_level((spdlog::level::level_enum)level);
	_getRuntimeLogger()->set_level((spdlog::level::level_enum)level);
}

std::shared_ptr<spdlog::logger> ayt::engine::log::Logger::_getPersistentLogger()
{
	return spdlog::get(_persistentLogger);
}

std::shared_ptr<spdlog::logger> ayt::engine::log::Logger::_getRuntimeLogger()
{
	return spdlog::get(_runtimeLogger);
}

void ayt::engine::log::Logger::_createLogger()
{
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

	if (_enableConsoleLog)
	{
		runtime.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		persistent.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	}

	auto persistentLogger = std::make_shared<spdlog::logger>(_persistentLogger, persistent.begin(), persistent.end());
	auto runtimeLogger = std::make_shared<spdlog::logger>(_runtimeLogger, runtime.begin(), runtime.end());
	spdlog::register_logger(persistentLogger);
	spdlog::register_logger(runtimeLogger);
	spdlog::set_default_logger(runtimeLogger);
}