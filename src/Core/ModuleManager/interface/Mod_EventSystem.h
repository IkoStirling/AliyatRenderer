#pragma once
#include <string>
#include <functional>
#include <memory>
#include "IAYModule.h"

class IAYEvent;
class AYEventToken;

class Mod_EventSystem : public IAYModule
{
public:
	//using EventHandler = std::function<void(const IAYEvent&)>;
public:
	//virtual void init() = 0;
	//virtual void update(float delta_time) = 0;
	//virtual void publish(std::unique_ptr<IAYEvent, PoolDeleter> in_event) = 0;
	//virtual AYEventToken* subscribe(const std::string& event_name, EventHandler event_callback) = 0;
	//virtual void unsubscribe(const std::string& event_name, EventHandler event_callback) = 0;
	//virtual void execute(std::shared_ptr<const IAYEvent> in_event) = 0;
	//virtual void executeJoin(std::unique_ptr<IAYEvent, PoolDeleter> in_event) = 0;
};