#include "AYEventToken.h"
#include "AYEventSystem.h"

AYEventToken::AYEventToken(AYEventSystem* system, const std::string& event_name, EventHandler event_callback) :
	_eventSystem(system),
	_eventName(event_name),
	_eventCallback(event_callback)
{
}

AYEventToken::~AYEventToken()
{
	_eventSystem->unsubscribe(_eventName, _eventCallback);
}