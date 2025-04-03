#include "AYEventSystem.h"
#include "AYEventThreadPoolManager.h"
#include "AYEventToken.h"
#include "IAYEvent.h"
#include <iostream>

AYEventSystem::AYEventSystem(std::unique_ptr<AYEventThreadPoolManager> in_manager):
	_eventManager(std::move(in_manager))
{
}

AYEventSystem::~AYEventSystem()
{
}

void AYEventSystem::setEventManager(std::unique_ptr<AYEventThreadPoolManager> in_manager)
{
	_eventManager = std::move(in_manager);
}

void AYEventSystem::update()
{
	_eventManager->update();
}

void AYEventSystem::publish(std::unique_ptr<IAYEvent> in_event)
{
	_eventManager->publish(std::move(in_event));
}

AYEventToken* AYEventSystem::subscribe(const std::string& event_name, EventHandler event_callback)
{
	_eventManager->subscribe(event_name, event_callback);
	return new AYEventToken(this, event_name, event_callback);
}

void AYEventSystem::unsubscribe(const std::string& event_name, EventHandler event_callback)
{
	_eventManager->unsubscribe(event_name, event_callback);
}

void AYEventSystem::execute(std::shared_ptr<const IAYEvent> in_event)
{
	_eventManager->execute(std::move(in_event));
}

void AYEventSystem::executeJoin(std::unique_ptr<IAYEvent> in_event)
{
	_eventManager->executeJoin(std::move(in_event));
}


