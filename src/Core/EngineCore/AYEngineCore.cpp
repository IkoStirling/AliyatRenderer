#include "Core/EventSystem/AYEventThreadPoolManager.h"
#include "Core/EventSystem/AYEventSystem.h"
#include "Core/MemoryPool/AYMemoryPool.h"
#include "Core/ResourceManager/AYResourceManager.h"
#include "AYEngineCore.h"

std::shared_ptr<AYEventSystem> AYEngineCore::getEventSystem()
{
	if (!_eventSystem)
		return nullptr;
	return _eventSystem;
}

AYEngineCore& AYEngineCore::getInstance()
{
	static AYEngineCore mInstance;
	return mInstance;
}

void AYEngineCore::init()
{
	AYMemoryPoolProxy::initMemoryPool();

	if (!_eventSystem)
	{
		auto tpm = std::make_unique<AYEventThreadPoolManager>();
		_eventSystem = std::make_shared<AYEventSystem>(std::move(tpm));
	}
}
void AYEngineCore::start()
{
	while (!_shouldClosed)
	{
		if (!update())
			break;
	}
}

bool AYEngineCore::update()
{
	GetEventSystem()->update();
	AYAsyncTracker::getInstance().update();
	return true;
}


AYEngineCore::AYEngineCore()
{
}

AYEngineCore::~AYEngineCore()
{
}