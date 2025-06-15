#pragma once
#include "IAYModule.h"

#define GetEngine() \
std::dynamic_pointer_cast<Mod_EngineCore>(AYModuleManager::getInstance().getModule("EngineCore"))

class Mod_EngineCore : public IAYModule 
{
public:
	virtual void setTargetFPS(float fps) = 0;

	virtual void setTimeScale(float scale) = 0;

	virtual float getCurrentFPS() const = 0;

	virtual float getDeltaTime() const = 0;

	virtual float getUnscaledDeltaTime() const = 0;
};