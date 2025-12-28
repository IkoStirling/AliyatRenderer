#pragma once
#include "IAYEvent.h"
#include "STTransform.h"
#include <iostream>
namespace ayt::engine::event
{
	class Event_CameraMove : public IEvent
	{
		DECLARE_EVENT_CLASS(Event_CameraMove, "Event_CameraMove")
		SUPPORT_MEMORY_POOL(Event_CameraMove)
	public:
		Event_CameraMove() :
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
				this->transform = static_cast<const Event_CameraMove&>(other).transform;
			}
		}

		math::Transform transform{};
	};

	REGISTER_EVENT_CLASS(Event_CameraMove);
}