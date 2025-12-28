#pragma once
#include <spdlog/spdlog.h>
#include <mutex>
#include <atomic>
#include <functional>

#define AYLOG(fmt, ...)  \
spdlog::info((fmt), ##__VA_ARGS__)

#define AYLOG_INFO(fmt, ...)  \
spdlog::info((fmt), ##__VA_ARGS__)

#define AYLOG_WARN(fmt, ...)  \
spdlog::warn((fmt), ##__VA_ARGS__)

#define AYLOG_ERR(fmt, ...)  \
spdlog::error((fmt), ##__VA_ARGS__)

#define AYLOG_CRIT(fmt, ...)  \
spdlog::critical((fmt), ##__VA_ARGS__)

#define AYLOG_TRACE(fmt, ...)  \
spdlog::trace((fmt), ##__VA_ARGS__)

namespace ayt::engine::log
{
	enum class LogLevel : int
	{
		trace = SPDLOG_LEVEL_TRACE,
		debug = SPDLOG_LEVEL_DEBUG,
		info = SPDLOG_LEVEL_INFO,
		warn = SPDLOG_LEVEL_WARN,
		err = SPDLOG_LEVEL_ERROR,
		critical = SPDLOG_LEVEL_CRITICAL,
		off = SPDLOG_LEVEL_OFF,
		n_levels
	};

	class Logger
	{
	public:
		Logger();
		~Logger();
	public:
		void init();
		void flush();
		void close();
		void setConsoleLog(bool enable);
		void setPattern(const std::string& pattern);
		void setLevel(LogLevel level);
//		void setPersistentLogMaxSize(size_t mb = 10);
	private:
		std::shared_ptr<spdlog::logger> _getPersistentLogger();
		std::shared_ptr<spdlog::logger> _getRuntimeLogger();
		void _createLogger();

		std::string _persistentLogger{ "persistent"};
		std::string _runtimeLogger{ "runtime"};
		std::atomic<bool> _enableConsoleLog{ true };
	};
}