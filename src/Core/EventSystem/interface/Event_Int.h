#pragma once
#include "IAYEvent.h"
#include <iostream>
namespace ayt::engine::event
{
	class Event_Int : public IEvent
	{
		DECLARE_EVENT_CLASS(Event_Int, "Event_Int")
		SUPPORT_MEMORY_POOL(Event_Int)
	public:
		Event_Int() :
			IEvent(Builder()
				.setPriority(99)
				.setMerge(true)
				.setLayer(EventLayer::GAME_LOGIC))
		{
		}

		virtual void merge(const IEvent& other) override
		{
			if (other.getTypeIndex() == this->getTypeIndex())
			{
				this->carryer += static_cast<const Event_Int&>(other).carryer;
			}
		}

		int carryer{ 0 };
	};

	REGISTER_EVENT_CLASS(Event_Int);
}