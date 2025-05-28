#include "AYModuleRegistry.h"
#include "IAYModule.h"

AYModuleManager& AYModuleManager::getInstance()
{
	static AYModuleManager mInstance;
	return mInstance;
}

bool AYModuleManager::registerModule(const std::string& name, std::shared_ptr<IAYModule> module)
{
	std::unique_lock<std::shared_mutex> lock(_moduleMutex);
	if (module)
	{
		_moduleMap[name] = std::move(module);
		return true;
	}
}

std::shared_ptr<IAYModule> AYModuleManager::getModule(const std::string& name) const
{
	std::shared_lock<std::shared_mutex> lock(_moduleMutex);
	auto it = _moduleMap.find(name);
	return it != _moduleMap.end() ? it->second : nullptr;
}

bool AYModuleManager::hasModule(const std::string& name) const
{
	std::shared_lock<std::shared_mutex> lock(_moduleMutex);
	return _moduleMap.find(name) != _moduleMap.end();
}

bool AYModuleManager::unregisterModule(const std::string& name)
{
	std::unique_lock<std::shared_mutex> lock(_moduleMutex);
	return _moduleMap.erase(name) > 0;
}

void AYModuleManager::allModuleInit()
{
	if (hasModule("EventSystem"))
		_moduleMap["EventSystem"]->init();

	std::vector<std::shared_ptr<IAYModule>> modules;
	{
		std::shared_lock<std::shared_mutex> lock(_moduleMutex);
		for (auto& [name, module] : _moduleMap) {
			if (module) modules.push_back(module);
		}
	}

	for (auto& module : modules) {
		module->init();
	}
}

void AYModuleManager::allModuleUpdate(float delta_time)
{
	std::shared_lock<std::shared_mutex> lock(_moduleMutex);
	for (auto it = _moduleMap.begin(); it != _moduleMap.end(); it++)
	{
		if (it->second)
			it->second->update(delta_time);
	}
}

