#pragma once
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
	double minExecTime;
	double maxExecTime;
	double avgExecTime;
	int count;
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
		_info->maxExecTime = duration.count();
		_info->minExecTime = duration.count();
		_info->avgExecTime = duration.count();
		_callback(_scopeName, std::unique_ptr<VDebugInfo>(std::move(_info)));
	}

private:
	std::chrono::steady_clock::time_point _start;

private:
	const std::string _scopeName;
	std::function<void(const std::string&, std::unique_ptr<VDebugInfo>)> _callback;
	VDebugInfo* _info;
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
				<< "show the infomation of " << scopeName << ": \n\n"
				<< std::setw(20) << "minExecTime: " 
				<< std::left << std::setw(20) << it->second->minExecTime << "s\n"
				<< std::setw(20) << "maxExecTime: "
				<< std::left << std::setw(20) << it->second->maxExecTime << "s\n"
				<< std::setw(20) << "avgExecTime: "
				<< std::left << std::setw(20) << it->second->avgExecTime << "s\n"
				<< std::setw(20) << "count: "
				<< std::left << std::setw(20) << it->second->count << "times\n\n"
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

			it->second->minExecTime =
				it->second->minExecTime > info->minExecTime ? info->minExecTime : it->second->minExecTime;
			it->second->maxExecTime =
				it->second->maxExecTime < info->maxExecTime ? info->maxExecTime : it->second->maxExecTime;
			it->second->avgExecTime =
				(it->second->avgExecTime * it->second->count + info->minExecTime) / (it->second->count + 1);
			it->second->count++;
		}
		else
		{
			info->count++;
			_infoMap[scopeName] = std::move(info);
		}
	}
private:
	std::unordered_map<std::string, std::unique_ptr<VDebugInfo>> _infoMap;
};
