#pragma once
#include "IAYEvent.h"
#include "STTransform.h"
#include <iostream>
class Event_CameraMove : public IAYEvent
{
	DECLARE_EVENT_CLASS(Event_CameraMove, "Event_CameraMove")
	SUPPORT_MEMORY_POOL(Event_CameraMove)
public:
	Event_CameraMove():
		IAYEvent(Builder()
		.setPriority(99)
		.setMerge(true)
		.setLayer(AYEventLayer::GAME_LOGIC))
	{}

	virtual void merge(const IAYEvent& other) override
	{
		if (other.getTypeIndex() == this->getTypeIndex())
		{
			this->transform = static_cast<const Event_CameraMove&>(other).transform;
		}
	}

	STTransform transform{};
};

REGISTER_EVENT_CLASS(Event_CameraMove);
