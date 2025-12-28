#pragma once
#include <string>
#include <functional>

namespace ayt::engine::event
{
	class EventSystem;
	class IEvent;

	class EventToken
	{
	public:
		using EventHandler = std::function<void(IEvent&)>;
	public:
		EventToken(EventSystem* system, const std::string& event_name, EventHandler event_callback);
		~EventToken();
	private:
		EventSystem* _eventSystem; //do not manage this ptr
		std::string _eventName;
		EventHandler _eventCallback;
	};
}