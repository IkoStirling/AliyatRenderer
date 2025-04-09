#pragma once
#include "ECEventDependence.h"
#include <memory>
#include "ResourceManager/MemoryPool/AYMemoryPool.h"
#include <typeindex> 


/*
	this marco's aim is to support events using memory pool,
	and provide some static methods
*/
#define DECLARE_EVENT_CLASS(CLASS_NAME, EVENT_TYPE_NAME) \
public: \
	virtual const char* getType()const override { \
		return EVENT_TYPE_NAME; \
	}\
	virtual std::type_index getTypeIndex()const override { \
		return typeid(CLASS_NAME); \
	}\
	virtual std::unique_ptr<IAYEvent> clone()const override{ \
		return std::make_unique<CLASS_NAME>(*this);\
	}\
	static const char* staticGetType(){ \
		return EVENT_TYPE_NAME; \
	}\
	static std::type_index staticGetTypeIndex(){ \
		return typeid(CLASS_NAME); \
	}\
public: \
	/* unused function */ \
	static std::shared_ptr<CLASS_NAME> UNUSED_create_shared() { \
		CLASS_NAME* ptr = new CLASS_NAME(); \
		return std::shared_ptr<CLASS_NAME>(ptr, [](CLASS_NAME* p) { \
			delete p; \
			}); \
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


/*
	不使用工厂方法，请避免使用make_shared绕开内存池
*/
class IAYEvent
{
	SUPPORT_MEMORY_POOL(IAYEvent)
public:
	class Builder;
public:
	explicit IAYEvent(const Builder& builder);

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

