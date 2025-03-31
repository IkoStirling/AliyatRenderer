#include "IAYEvent.h"

bool IAYEvent::operator<(const IAYEvent& other) const
{
	return  priority < other.priority;
}

bool IAYEvent::operator>(const IAYEvent& other) const
{
	return  priority > other.priority;
}

IAYEvent::Builder::Builder() :
	priority(99),
	shouldMerge(false),
	layer(AYEventLayer::GAME_LOGIC)
{

}

IAYEvent::Builder& IAYEvent::Builder::setPriority(int in_priority)
{
	this->priority = in_priority;
	return *this;
}

IAYEvent::Builder& IAYEvent::Builder::setMerge(bool in_tickOnce)
{
	this->shouldMerge = in_tickOnce;
	return *this;
}

IAYEvent::Builder& IAYEvent::Builder::setLayer(AYEventLayer in_layer)
{
	this->layer = in_layer;
	return *this;
}