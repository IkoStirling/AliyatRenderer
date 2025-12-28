#pragma once
#include "IAYEvent.h"
#include "BasePhy/IAYPhysical.h"
namespace ayt::engine::physics
{
	class Event_PhysicsBeginContact : public ayt::engine::event::IEvent
	{
		DECLARE_EVENT_CLASS(Event_PhysicsBeginContact, "Event_PhysicsBeginContact")
		SUPPORT_MEMORY_POOL(Event_PhysicsBeginContact)
	public:
		Event_PhysicsBeginContact() :
			IEvent(Builder()
				.setPriority(50)	//暂定中等优先级
				.setMerge(false)	//碰撞事件不可合并
				.setLayer(EventLayer::PHYSICS))
		{
		}

		virtual void merge(const IEvent& other) override
		{
			if (other.getTypeIndex() == this->getTypeIndex())
			{
				this->carryer += static_cast<const Event_Int&>(other).carryer;
			}
		}

		/*IAYPhysical* entityA;
		IAYPhysical* entityB;*/
	};

	REGISTER_EVENT_CLASS(Event_PhysicsBeginContact);
}