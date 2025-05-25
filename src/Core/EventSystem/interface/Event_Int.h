#pragma once
#include "IAYEvent.h"
#include <iostream>
class Event_Int : public IAYEvent
{
	DECLARE_EVENT_CLASS(Event_Int, "Event_Int")
	SUPPORT_MEMORY_POOL(Event_Int)
public:
	Event_Int():
		IAYEvent(Builder()
		.setPriority(99)
		.setMerge(true)
		.setLayer(AYEventLayer::GAME_LOGIC))
	{}

	virtual void merge(const IAYEvent& other) override
	{
		if (other.getTypeIndex() == this->getTypeIndex())
		{
			this->carryer += static_cast<const Event_Int&>(other).carryer;
		}
	}

	int carryer{0};
};

REGISTER_EVENT_CLASS(Event_Int);
