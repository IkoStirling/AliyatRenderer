#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>

#define GET_MODULE(MODULE_NAME) \
	::ayt::engine::modules::ModuleManager::getInstance().getModule(MODULE_NAME)

#define GET_CAST_MODULE(CAST_TYPE, MODULE_NAME) \
	std::dynamic_pointer_cast<CAST_TYPE>(::ayt::engine::modules::ModuleManager::getInstance().getModule(MODULE_NAME))

namespace ayt::engine::modules
{
	class IModule;

	class ModuleManager
	{
	public:
		static ModuleManager& getInstance();

		bool registerModule(const std::string& name, std::shared_ptr<IModule> module);

		std::shared_ptr<IModule> getModule(const std::string& name) const;

		bool hasModule(const std::string& name) const;

		bool unregisterModule(const std::string& name);

		void allModuleInit();	//该函数已废弃

		void allModuleUpdate(float delta_time); //该函数已废弃

		void allModuleShutdown();

	private:
		ModuleManager() = default;
		ModuleManager(const ModuleManager&) = delete;
		ModuleManager& operator=(const ModuleManager&) = delete;

	private:
		std::unordered_map<std::string, std::shared_ptr<IModule>> _moduleMap;
		mutable std::shared_mutex _moduleMutex; //mutable 让const方法可以使用锁
	};
}
