#include "AYEventToken.h"
#include "AYEventSystem.h"
namespace ayt::engine::event
{
	EventToken::EventToken(EventSystem* system, const std::string& event_name, EventHandler event_callback) :
		_eventSystem(system),
		_eventName(event_name),
		_eventCallback(event_callback)
	{
	}

	EventToken::~EventToken()
	{
		_eventSystem->unsubscribe(_eventName, _eventCallback);
	}
}