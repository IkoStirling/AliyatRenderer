#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>


class IAYModule;

class AYModuleManager
{
public:
	static AYModuleManager& getInstance();

	void registerModule(const std::string& name, std::shared_ptr<IAYModule> module);

	std::shared_ptr<IAYModule> getModule(const std::string& name) const;

	bool hasModule(const std::string& name) const;

	bool unregisterModule(const std::string& name);

	void allModuleInit();

	void allModuleUpdate();
private:
	AYModuleManager() = default;
	AYModuleManager(const AYModuleManager&) = delete;
	AYModuleManager& operator=(const AYModuleManager&) = delete;

private:
	std::unordered_map<std::string, std::shared_ptr<IAYModule>> _moduleMap;
	mutable std::shared_mutex _moduleMutex; //mutable 让const方法可以使用锁
};