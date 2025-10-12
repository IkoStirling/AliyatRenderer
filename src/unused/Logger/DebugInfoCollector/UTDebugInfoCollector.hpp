#pragma once
#include "Core/MemoryPool/AYMemoryPool.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <functional>
#include <chrono>
#include <iomanip>

#define DEBUG_COLLECT() std::unique_ptr<UTDebugInfoAgent> __unnamed__ = \
UTDebugInfoCollector::getInfoCollector().collectThis(__func__);

#define DEBUG_CONSOLE_SHOW(param) UTDebugInfoCollector::getInfoCollector().show(#param)

class VDebugInfo
{
public:
	double curExecTime;
public:
	SUPPORT_MEMORY_POOL(VDebugInfo)
};

struct CollectedInfo
{
	double minExecTime;
	double maxExecTime;
	double totalExecTime;
	int count;

	CollectedInfo():
		minExecTime(0),
		maxExecTime(0),
		totalExecTime(0),
		count(1)
	{

	}
	CollectedInfo(VDebugInfo* in_info):
		minExecTime(in_info->curExecTime),
		maxExecTime(in_info->curExecTime),
		totalExecTime(in_info->curExecTime),
		count(1)
	{ }
};

class UTDebugInfoAgent
{
public:
	UTDebugInfoAgent(
		const std::string& scopeName,
		std::function<void(const std::string&, std::unique_ptr<VDebugInfo>)> callback) :
		_scopeName(scopeName), _callback(callback)
	{
		_info = new VDebugInfo();
		_start = std::chrono::high_resolution_clock::now();
	}
	~UTDebugInfoAgent()
	{
		auto _end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = _end - _start;
		_info->curExecTime = duration.count();
		_callback(_scopeName, std::unique_ptr<VDebugInfo>(std::move(_info)));
	}

private:
	std::chrono::steady_clock::time_point _start;

private:
	const std::string _scopeName;
	std::function<void(const std::string&, std::unique_ptr<VDebugInfo>)> _callback;
	VDebugInfo* _info;
public:
	SUPPORT_MEMORY_POOL(UTDebugInfoAgent)
};

class UTDebugInfoCollector
{
public:
	std::unique_ptr<UTDebugInfoAgent> collectThis(const std::string& scopeName)
	{
		return std::make_unique<UTDebugInfoAgent>(
			scopeName,
			std::bind(
				&UTDebugInfoCollector::doCollect,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
			);
	}
	static UTDebugInfoCollector& getInfoCollector()
	{
		static UTDebugInfoCollector infoCollector;
		return infoCollector;
	}

	void show(const std::string& scopeName)
	{
		auto it = _infoMap.find(scopeName);
		if (it != _infoMap.end())
		{
			std::cout
				<< std::fixed << std::setprecision(10)
				<< "show the infomation of " << scopeName << ": \n" << std::endl
				<< std::left << std::setw(20) << "minExecTime: "
				<< std::left << std::setw(20) << it->second.minExecTime << "s\n"
				<< std::setw(20) << "maxExecTime: "
				<< std::left << std::setw(20) << it->second.maxExecTime << "s\n"
				<< std::setw(20) << "totalExecTime: "
				<< std::left << std::setw(20) << it->second.totalExecTime << "s\n"
				<< std::setw(20) << "avgExecTime: "
				<< std::left << std::setw(20) << it->second.totalExecTime / it->second.count << "s\n"
				<< std::setw(20) << "count: "
				<< std::left << std::setw(20) << it->second.count << "times\n\n"
				;
		}
	}


private:
	UTDebugInfoCollector() {};
	void doCollect(const std::string& scopeName, std::unique_ptr<VDebugInfo> info)
	{
		auto it = _infoMap.find(scopeName);
		if (it != _infoMap.end())
		{

			it->second.minExecTime =
				it->second.minExecTime > info->curExecTime ? info->curExecTime : it->second.minExecTime;
			it->second.maxExecTime =
				it->second.maxExecTime < info->curExecTime ? info->curExecTime : it->second.maxExecTime;
			it->second.totalExecTime = it->second.totalExecTime + info->curExecTime;
			it->second.count++;
		}
		else
		{
			_infoMap[scopeName] = CollectedInfo(info.get());
		}
	}
private:
	std::unordered_map<std::string, CollectedInfo> _infoMap;
	std::string _cache;
};
