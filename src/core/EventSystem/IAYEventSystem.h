#pragma once
#include <string>
#include <functional>
#include <memory>

class IAYEvent;
class AYEventToken;

class IAYEventSystem
{
public:
	using EventHandler = std::function<void(const IAYEvent&)>;
public:
	virtual void update() = 0;
	virtual void publish(std::unique_ptr<IAYEvent> in_event) = 0;
	virtual AYEventToken* subscribe(const std::string& event_name, EventHandler event_callback) = 0;
	virtual void unsubscribe(const std::string& event_name, EventHandler event_callback) = 0;
	virtual void execute(std::shared_ptr<const IAYEvent> in_event) = 0;
};