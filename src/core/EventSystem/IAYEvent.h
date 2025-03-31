#pragma once
#include "ECEventDependence.h"
#include <memory>
class IAYEvent
{
public:
	class Builder;
public:
	IAYEvent(Builder builder) :
		priority(builder.priority),
		shouldMerge(builder.shouldMerge),
		layer(builder.layer)
	{}

	virtual ~IAYEvent() = default;
	virtual const char* getType()const = 0;
	virtual std::unique_ptr<IAYEvent> clone()const = 0;
	virtual void merge(const IAYEvent& other) = 0;

	bool operator<(const IAYEvent& other) const;
	bool operator>(const IAYEvent& other) const;
	
public:
	const int priority;
	const bool shouldMerge;
	const AYEventLayer layer;

public:
	class Builder
	{
	public:
		Builder();

		Builder& setPriority(int in_priority);
		Builder& setMerge(bool in_tickOnce);
		Builder& setLayer(AYEventLayer in_layer);

		int priority;
		bool shouldMerge;
		AYEventLayer layer;
	};
};