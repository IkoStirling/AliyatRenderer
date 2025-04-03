#pragma once
#include "../IAYEvent.h"

class Event_Int : public IAYEvent
{
	DECLARE_EVENT_CLASS(Event_Int, "Event_Int");
public:
	Event_Int():
		IAYEvent(Builder()
		.setPriority(99)
		.setMerge(true)
		.setLayer(AYEventLayer::GAME_LOGIC))
	{}

	virtual void merge(const IAYEvent& other) override
	{
		if (other.getTypeIndex == this->getTypeIndex())
		{
			this->carryer += static_cast<const EventWithInt&>(other).carryer;
		}
	}

	virtual std::unique_ptr<IAYEvent> clone()const override
	{
		return std::make_unique<Event_Int>();
	}

	int carryer;
};

REGISTER_EVENT_CLASS(Event_Int);
