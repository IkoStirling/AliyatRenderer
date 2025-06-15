#include <iostream>
#include <Core/EngineCore/AYEngineCore.h>
#include "Core/EventSystem/AYEventSystem.h"
#include "Core/SystemEventType/Event_ResourceLoadAsync.h"
#include "Core/SystemResourceType/AYTexture.h"
#include "Core/ResourceManager/AYResourceManager.h"


namespace ResourceManager {

	void benchmark_ResourceCount()
	{
		{
			auto x = AYResourceManager::getInstance().loadAsync<AYTexture>("12345");
		}
		GetEventSystem()->update();
		std::this_thread::sleep_for(std::chrono::seconds(2));
		AYResourceManager::getInstance().printStats();

		AYAsyncTracker::getInstance().update();
		AYResourceManager::getInstance().printStats();	
	}

	void benchmark_Tags()
	{
		//已经缓存过的资源直接从缓存中获取
		auto x = AYResourceManager::getInstance().load<AYTexture>("12345");
		AYResourceManager::getInstance().tagResource("12345", "test");
		AYResourceManager::getInstance().printTaggedStats("test");

		AYResourceManager::getInstance().unpinResource("12345");
		AYResourceManager::getInstance().printStats();
	}

	void benchmark_Persistent()
	{
		AYResourceManager::getInstance().printStats();
		auto x = AYResourceManager::getInstance().load<AYTexture>("12345");
		
	}
}

int main()
{
	GetEngine().init();
	//ResourceManager::benchmark_ResourceCount();
	//ResourceManager::benchmark_Tags();
	ResourceManager::benchmark_Persistent();
	return 0;
}