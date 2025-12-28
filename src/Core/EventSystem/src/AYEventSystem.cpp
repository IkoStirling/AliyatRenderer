#include <iostream>
#include "AYEventSystem.h"
#include "AYEventToken.h"
#include "IAYEvent.h"

namespace ayt::engine::event
{
	EventSystem::EventSystem(std::unique_ptr<EventPool> in_manager) :
		_eventManager(std::move(in_manager))
	{
	}

	EventSystem::~EventSystem()
	{
	}

	void EventSystem::setEventManager(std::unique_ptr<EventPool> in_manager)
	{
		_eventManager = std::move(in_manager);
	}

	void EventSystem::init()
	{
		if (!_eventManager)
			setEventManager(std::make_unique<EventPool>());
	}

	void EventSystem::update(float delta_time)
	{
		_eventManager->update();
	}

	void EventSystem::shutdown()
	{
		_eventManager->shutdown();
	}

	void EventSystem::publish(std::unique_ptr<IEvent, PoolDeleter> in_event)
	{
		_eventManager->publish(std::move(in_event));
	}

	EventToken* EventSystem::subscribe(const std::string& event_name, EventHandler event_callback)
	{
		_eventManager->subscribe(event_name, event_callback);
		return new EventToken(this, event_name, event_callback);
	}

	void EventSystem::unsubscribe(const std::string& event_name, EventHandler event_callback)
	{
		_eventManager->unsubscribe(event_name, event_callback);
	}

	void EventSystem::execute(std::shared_ptr<IEvent> in_event)
	{
		_eventManager->execute(std::move(in_event));
	}

	void EventSystem::executeJoin(std::unique_ptr<IEvent, PoolDeleter> in_event)
	{
		_eventManager->executeJoin(std::move(in_event));
	}


}