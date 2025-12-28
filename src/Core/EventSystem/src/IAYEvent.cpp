#include "IAYEvent.h"
#include <algorithm>
namespace ayt::engine::event
{
	IEvent::Builder::Builder() :
		priority(99),
		shouldMerge(false),
		layer(EventLayer::GAME_LOGIC)
	{

	}

	IEvent::Builder& IEvent::Builder::setPriority(int in_priority)
	{
		this->priority = std::clamp(in_priority, 0, 100);
		return *this;
	}

	IEvent::Builder& IEvent::Builder::setMerge(bool in_tickOnce)
	{
		this->shouldMerge = in_tickOnce;
		return *this;
	}

	IEvent::Builder& IEvent::Builder::setLayer(EventLayer in_layer)
	{
		this->layer = in_layer;
		return *this;
	}

	IEvent::IEvent(const Builder& builder) :
		priority(builder.priority),
		shouldMerge(builder.shouldMerge),
		layer(builder.layer)
	{

	}
}