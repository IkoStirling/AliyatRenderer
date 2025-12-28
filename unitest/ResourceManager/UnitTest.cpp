#include <iostream>
#include <AYEngineCore.h>
#include "AYEventSystem.h"
#include "Event_ResourceLoadAsync.h"
#include "AYTexture.h"
#include "AYResourceManager.h"


namespace ResourceManager {

	void benchmark_ResourceCount()
	{
		{
			auto x = ResourceManager::getInstance().loadAsync<AYTexture>("12345");
		}
		ModuleManager::getInstance().getModule("EventSystem")->update(1);
		std::this_thread::sleep_for(std::chrono::seconds(2));
		ResourceManager::getInstance().printStats();

		AsyncTracker::getInstance().update(1);
		ResourceManager::getInstance().printStats();	
	}

	void benchmark_Tags()
	{
		//已经缓存过的资源直接从缓存中获取
		auto x = ResourceManager::getInstance().load<AYTexture>("12345");
		ResourceManager::getInstance().tagResource("12345", "test");
		ResourceManager::getInstance().printTaggedStats("test");

		ResourceManager::getInstance().unpinResource("12345");
		ResourceManager::getInstance().printStats();
	}

	void benchmark_Persistent()
	{
		ResourceManager::getInstance().printStats();
		auto x = ResourceManager::getInstance().load<AYTexture>("12345");
		
	}
}

int main()
{
	AYEngineCore::getInstance().init();
	//AYEngineCore::getInstance().start();
	ResourceManager::benchmark_ResourceCount();
	ResourceManager::benchmark_Tags();
	ResourceManager::benchmark_Persistent();
	return 0;
}