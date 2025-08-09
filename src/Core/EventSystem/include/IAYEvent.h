#pragma once
#include "ECEventDependence.h"
#include "AYMemoryPool.h"
#include <memory>
#include <typeindex> 
#include <string>


/*
	this marco's aim is to provide some default methods
*/
#define DECLARE_EVENT_CLASS(CLASS_NAME, EVENT_TYPE_NAME) \
public: \
	virtual const char* getType()const override { \
		static std::string str = EVENT_TYPE_NAME; \
		return str.c_str(); \
	}\
	virtual std::type_index getTypeIndex()const override { \
		return typeid(CLASS_NAME); \
	}\
	static const char* staticGetType(){ \
		static std::string str = EVENT_TYPE_NAME; \
		return str.c_str(); \
	}\
	static std::type_index staticGetTypeIndex(){ \
		return typeid(CLASS_NAME); \
	}\
private:\

/*
	this marco's aim is to provide some default methods
*/
#define DECLARE_TEMPLATE_EVENT_CLASS(CLASS_NAME, EVENT_TYPE_NAME, TEMPLATE_TYPE_NAME) \
public: \
	virtual const char* getType()const override { \
		static std::string str = EVENT_TYPE_NAME + std::string(typeid(TEMPLATE_TYPE_NAME).name()); \
		return str.c_str(); \
	}\
	virtual std::type_index getTypeIndex()const override { \
		return typeid(CLASS_NAME<TEMPLATE_TYPE_NAME>); \
	}\
	static const char* staticGetType(){ \
		static std::string str = EVENT_TYPE_NAME+ std::string(typeid(TEMPLATE_TYPE_NAME).name()); \
		return str.c_str(); \
	}\
	static std::type_index staticGetTypeIndex(){ \
		return typeid(CLASS_NAME<TEMPLATE_TYPE_NAME>); \
	}\
private:\


#define REGISTER_EVENT_CLASS(CLASS_NAME, ...) \
struct CLASS_NAME##_Register{ \
	CLASS_NAME##_Register(){ \
		::AYEventRegistry::getInstance().registerEvent<CLASS_NAME>(CLASS_NAME::staticGetType()); \
	}\
}; \
static CLASS_NAME##_Register CLASS_NAME##_register; \


#define REGISTER_TEMPLATE_EVENT_CLASS(CLASS_NAME, TYPE_NAME) \
struct CLASS_NAME##_Register##TYPE_NAME{ \
	CLASS_NAME##_Register##TYPE_NAME(){ \
		::AYEventRegistry::getInstance().registerEvent<CLASS_NAME<TYPE_NAME>>(CLASS_NAME<TYPE_NAME>::staticGetType()); \
	}\
}; \
static CLASS_NAME##_Register##TYPE_NAME CLASS_NAME##_register##TYPE_NAME; \




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

	virtual void merge(const IAYEvent& other) {};
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

		int priority;	//数值越小优先级越高，取值[0,100]
		bool shouldMerge;
		AYEventLayer layer;
	};
public:
	virtual ~IAYEvent() = default;

	virtual const char* getType()const = 0;
	virtual std::type_index getTypeIndex()const = 0;
};

