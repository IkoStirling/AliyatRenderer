#include "AYEngineCore.h"


AYEngineCore& AYEngineCore::getInstance()
{
	static AYEngineCore mInstance;
	return mInstance;
}

void AYEngineCore::init()
{
	AYModuleManager::getInstance().allModuleInit();
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
	AYModuleManager::getInstance().allModuleUpdate();
	return true;
}

AYEngineCore::~AYEngineCore()
{
}