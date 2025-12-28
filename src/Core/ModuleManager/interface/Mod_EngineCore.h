#pragma once
#include "IAYModule.h"

#define GetEngine() \
std::dynamic_pointer_cast<::ayt::engine::Mod_EngineCore>(::ayt::engine::modules::ModuleManager::getInstance().getModule("EngineCore"))

namespace ayt::engine
{
	class Mod_EngineCore : public ::ayt::engine::modules::IModule
	{
	public:
		virtual void setTargetFPS(float fps) = 0;

		virtual void setTimeScale(float scale) = 0;

		virtual float getCurrentFPS() const = 0;

		virtual float getDeltaTime() const = 0;

		virtual float getUnscaledDeltaTime() const = 0;
	};
}
