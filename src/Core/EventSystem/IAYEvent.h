#pragma once
#include "ECEventDependence.h"
#include <memory>
#include <typeindex> 

#define HAS_ARGS(...)  (sizeof((int[]){__VA_ARGS__}) > 1)

#define DECLARE_EVENT_CLASS(CLASS_NAME, EVENT_TYPE_NAME) \
public: \
	virtual const char* getType()const override { \
		return EVENT_TYPE_NAME; \
	}\
	virtual std::type_index getTypeIndex()const override { \
		return typeid(CLASS_NAME); \
	}\
	static const char* staticGetType(){ \
		return EVENT_TYPE_NAME; \
	}\
	static std::type_index staticGetTypeIndex(){ \
		return typeid(CLASS_NAME); \
	}\
	virtual std::unique_ptr<IAYEvent> clone()const override \
	{ \
		return std::make_unique<CLASS_NAME>(*this); \
	}\
private:\


#define REGISTER_EVENT_CLASS(CLASS_NAME, ...) \
struct CLASS_NAME##_Register{ \
	CLASS_NAME##_Register(){ \
		::AYEventRegistry::getInstance().registerEvent<CLASS_NAME>(CLASS_NAME::staticGetType()); \
	}\
}; \
static CLASS_NAME##_Register CLASS_NAME##_register; \


#define GENERATE_EVENT_CONSTRUCTOR(CLASS_NAME, ...) \
public: \
	CLASS_NAME(__VA_ARGS__)



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
	virtual void merge(const IAYEvent& other) = 0;
	virtual std::unique_ptr<IAYEvent> clone()const = 0;
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
public:
	virtual ~IAYEvent() = default;

	virtual const char* getType()const = 0;
	virtual std::type_index getTypeIndex()const = 0;
};

