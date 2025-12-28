#include "AYModuleRegistry.h"
#include "IAYModule.h"
namespace ayt::engine::modules
{
	ModuleManager& ModuleManager::getInstance()
	{
		static ModuleManager mInstance;
		return mInstance;
	}

	bool ModuleManager::registerModule(const std::string& name, std::shared_ptr<IModule> module)
	{
		std::unique_lock<std::shared_mutex> lock(_moduleMutex);
		if (module)
		{
			_moduleMap[name] = std::move(module);
			return true;
		}
		return false;
	}

	std::shared_ptr<IModule> ModuleManager::getModule(const std::string& name) const
	{
		std::shared_lock<std::shared_mutex> lock(_moduleMutex);
		auto it = _moduleMap.find(name);
		return it != _moduleMap.end() ? it->second : nullptr;
	}

	bool ModuleManager::hasModule(const std::string& name) const
	{
		std::shared_lock<std::shared_mutex> lock(_moduleMutex);
		return _moduleMap.find(name) != _moduleMap.end();
	}

	bool ModuleManager::unregisterModule(const std::string& name)
	{
		std::unique_lock<std::shared_mutex> lock(_moduleMutex);
		return _moduleMap.erase(name) > 0;
	}

	void ModuleManager::allModuleInit()
	{
		std::vector<std::shared_ptr<IModule>> modules;
		{
			std::shared_lock<std::shared_mutex> lock(_moduleMutex);

			if (hasModule("MemoryPool"))
				_moduleMap["MemoryPool"]->init();

			if (hasModule("EventSystem"))
				_moduleMap["EventSystem"]->init();

			for (auto& [name, module] : _moduleMap) {
				if (module) modules.push_back(module);
			}
		}

		for (auto& module : modules) {
			module->init();
		}
	}

	void ModuleManager::allModuleUpdate(float delta_time)
	{
		std::shared_lock<std::shared_mutex> lock(_moduleMutex);
		for (auto it = _moduleMap.begin(); it != _moduleMap.end(); it++)
		{
			if (it->second)
				it->second->update(delta_time);
		}
	}

	void ModuleManager::allModuleShutdown()
	{
		std::unique_lock<std::shared_mutex> lock(_moduleMutex);
		for (auto it = _moduleMap.begin(); it != _moduleMap.end(); it++)
		{
			if (it->second)
				it->second->shutdown();
		}
	}
}