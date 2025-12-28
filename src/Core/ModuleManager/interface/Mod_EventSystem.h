#pragma once
#include <string>
#include <functional>
#include <memory>
#include "IAYModule.h"
namespace ayt::engine::event
{
	class IEvent;
	class EventToken;

	class Mod_EventSystem : public ayt::engine::modules::IModule
	{
	public:
		//using EventHandler = std::function<void(const IEvent&)>;
	public:
		//virtual void init() = 0;
		//virtual void update(float delta_time) = 0;
		//virtual void publish(std::unique_ptr<IEvent, PoolDeleter> in_event) = 0;
		//virtual EventToken* subscribe(const std::string& event_name, EventHandler event_callback) = 0;
		//virtual void unsubscribe(const std::string& event_name, EventHandler event_callback) = 0;
		//virtual void execute(std::shared_ptr<const IEvent> in_event) = 0;
		//virtual void executeJoin(std::unique_ptr<IEvent, PoolDeleter> in_event) = 0;
	};
}