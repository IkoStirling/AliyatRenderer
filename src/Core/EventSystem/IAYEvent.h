#pragma once
#include "ECEventDependence.h"
#include <memory>
#include "ResourceManager/MemoryPool/AYMemoryPool.h"
#include <typeindex> 



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
		void* mem = AYMemoryPoolProxy::useMemoryPool(sizeof(CLASS_NAME));\
		if (!mem) throw std::bad_alloc();\
		\
		CLASS_NAME* newObj = new (mem) CLASS_NAME(*this); \
		return std::unique_ptr<IAYEvent>(newObj);\
	}\
	void* operator new(size_t size){ \
		void* ptr = AYMemoryPoolProxy::useMemoryPool(size); \
		if (!ptr) throw std::bad_alloc(); \
		memset(ptr, 0, size); \
		return ptr; \
	} \
	void operator delete(void* ptr){ \
		DeleteObject<CLASS_NAME>(static_cast<CLASS_NAME*>(ptr)); \
	} \
	static std::shared_ptr<CLASS_NAME> create_shared() { \
		CLASS_NAME* ptr = new CLASS_NAME(); \
		return std::shared_ptr<CLASS_NAME>(ptr, [](CLASS_NAME* p) { \
			delete p; \
			}); \
	} \
    static void* operator new(size_t size, void* ptr) noexcept { \
		return ::operator new(size, ptr); \
	} \
private:\
	static void* operator new(size_t size, const std::nothrow_t&) noexcept = delete;


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

