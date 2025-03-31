#pragma once
#include "IAYEventSystem.h"
#include <functional>

// CALLBACK_WITH_CLASS_NAME : like MyClass::func
#define SUBSCRIBE_EVENT(EVENT_SYSTEM,EVENT_NAME,CALLBACK_WITH_CLASS_NAME) \
	EVENT_SYSTEM->subscribe(EVENT_NAME,std::bind(&CALLBACK_WITH_CLASS_NAME,this,std::placeholders::_1))

class IAYEvent;
class AYEventThreadPoolManager;
class AYEventToken;

class AYEventSystem : public IAYEventSystem
{
public:
	using EventHandler = std::function<void(const IAYEvent&)>;
public:

	AYEventSystem();

	~AYEventSystem();

	void update() override;
	void publish(std::unique_ptr<IAYEvent> in_event) override;
	AYEventToken* subscribe(const std::string& event_name, EventHandler event_callback) override;
	void unsubscribe(const std::string& event_name, EventHandler event_callback) override;
	void execute(std::shared_ptr<const IAYEvent> in_event) override;

private:
	std::unique_ptr<AYEventThreadPoolManager> _eventManager;

};