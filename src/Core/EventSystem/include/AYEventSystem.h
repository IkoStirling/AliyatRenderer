#pragma once
#include "Mod_EventSystem.h"
#include "AYEventThreadPoolManager.h"

// CALLBACK_WITH_CLASS_NAME : like MyClass::func
#define SUBSCRIBE_EVENT(EVENT_SYSTEM,EVENT_NAME,CALLBACK_WITH_CLASS_NAME) \
	EVENT_SYSTEM->subscribe(EVENT_NAME,std::bind(&CALLBACK_WITH_CLASS_NAME,this,std::placeholders::_1))

#define SUBSCRIBE_EVENT_LAMBDA(EVENT_SYSTEM,EVENT_NAME,CALLBACK_WITH_CLASS_NAME) \
	EVENT_SYSTEM->subscribe(EVENT_NAME,CALLBACK_WITH_CLASS_NAME)

class IAYEvent;
class AYEventToken;

/*
	该类作用：
		事件系统
		发布事件
		对于发布的事件进行订阅
*/
class AYEventSystem : public Mod_EventSystem
{
public:
	using EventHandler = std::function<void(const IAYEvent&)>;
public:
	AYEventSystem() = default;
	AYEventSystem(std::unique_ptr<AYEventThreadPoolManager> in_manager);
	~AYEventSystem();

	void setEventManager(std::unique_ptr<AYEventThreadPoolManager> in_manager);

	void init() override;
	void update(float delta_time) override;
	void publish(std::unique_ptr<IAYEvent> in_event) override;
	AYEventToken* subscribe(const std::string& event_name, EventHandler event_callback) override;
	void unsubscribe(const std::string& event_name, EventHandler event_callback) override;
	void execute(std::shared_ptr<const IAYEvent> in_event) override;
	void executeJoin(std::unique_ptr<IAYEvent> in_event) override;

private:
	std::unique_ptr<AYEventThreadPoolManager> _eventManager;

};

REGISTER_MODULE_CLASS("EventSystem", AYEventSystem)
