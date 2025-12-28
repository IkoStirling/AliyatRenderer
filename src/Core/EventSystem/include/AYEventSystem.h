#pragma once
#include "Mod_EventSystem.h"
#include "AYEventThreadPoolManager.h"


namespace ayt::engine::event
{

#ifdef NDEBUG
	// CALLBACK_WITH_CLASS_NAME : like MyClass::func
#define SUBSCRIBE_EVENT(EVENT_SYSTEM,EVENT_NAME,CALLBACK_WITH_CLASS_NAME) \
	EVENT_SYSTEM->subscribe(EVENT_NAME,std::bind(&CALLBACK_WITH_CLASS_NAME,this,std::placeholders::_1))

#define SUBSCRIBE_EVENT_LAMBDA(EVENT_SYSTEM,EVENT_NAME,CALLBACK_WITH_CLASS_NAME) \
	EVENT_SYSTEM->subscribe(EVENT_NAME,CALLBACK_WITH_CLASS_NAME)

#endif // NDEBUG


	class IEvent;
	class EventToken;

	/*
		该类作用：
			事件系统
			发布事件
			对于发布的事件进行订阅

		注意：
			事件倾向于使用EventRegistry类进行publish调用！
	*/
	class EventSystem : public Mod_EventSystem
	{
	public:
		using EventHandler = std::function<void(IEvent&)>;
	public:
		EventSystem() = default;
		EventSystem(std::unique_ptr<EventPool> in_manager);
		~EventSystem();

		void setEventManager(std::unique_ptr<EventPool> in_manager);

		void init() override;
		void update(float delta_time) override;
		void shutdown() override;
		void publish(std::unique_ptr<IEvent, PoolDeleter> in_event);
		EventToken* subscribe(const std::string& event_name, EventHandler event_callback);
		void unsubscribe(const std::string& event_name, EventHandler event_callback);
		void execute(std::shared_ptr<IEvent> in_event);
		void executeJoin(std::unique_ptr<IEvent, PoolDeleter> in_event);

	private:
		std::unique_ptr<EventPool> _eventManager;

	};

	REGISTER_MODULE_CLASS("EventSystem", EventSystem)

}