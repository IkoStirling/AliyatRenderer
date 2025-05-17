#pragma once
#include <string>
#include <functional>

class AYEventSystem;
class IAYEvent;

class AYEventToken
{
public:
	using EventHandler = std::function<void(const IAYEvent&)>;
public:
	AYEventToken(AYEventSystem* system, const std::string& event_name, EventHandler event_callback);
	~AYEventToken();
private:
	AYEventSystem* _eventSystem; //do not manage this ptr
	std::string _eventName;
	EventHandler _eventCallback;
};