#pragma once
#include "Mod_MemoryPool.h"
#include <iostream>
#include <atomic>
#include <mutex>

#define MAX_SLOT_SIZE 512
#define SLOT_BASE_SIZE 8
#define POOL_NUMS 64

#define SUPPORT_MEMORY_POOL(CLASS_NAME) \
public: \
	void* operator new(size_t size){ \
		void* ptr = AYMemoryPoolProxy::useMemoryPool(size); \
		if (!ptr) throw std::bad_alloc(); \
		memset(ptr, 0, size); \
		return ptr; \
	} \
	void operator delete(void* ptr){ \
		AYMemoryPoolProxy::freeMomory(reinterpret_cast<void*>(ptr), sizeof(CLASS_NAME)); \
	} \
	static void* operator new(size_t size, void* ptr) noexcept { \
		return ::operator new(size, ptr); \
	} \
private: \



struct Slot
{
	std::atomic<Slot*> next;
};

/*
	每个内存块首部包含slot指针，用于索引下一个内存块
	除此之外，实际存储对象的插槽，并不包含slot指针，仅由内存池主动管理（计算偏移）

	注意：！！！
	使用原版智能指针将会无视上述重载的new和delete，智能指针使用的是全局的new和delete
	目前内存池实现几乎完备，经测试可以使用
*/
class AYMemoryPool
{
public:
	AYMemoryPool(size_t in_blockSize=4096);
	~AYMemoryPool();

	void init(size_t in_slotSize);
	void* allocate();
	void deallocate(void*);

private:
	void newBlock();
	size_t paddingBlock(char* ptr, size_t align);

	bool pushFreeSlot(Slot* slot);
	Slot* popFreeSlot();
private:
	uint32_t _blockSize; //内存块大小
	uint32_t _slotSize; //实际插槽大小
	Slot* _firstBlock; //指向最新的内存块，链表
	Slot* _curSlot; //当前内存块标记位置，创建内存块会被更新
	std::atomic<Slot*> _freeSlots; //所有线程及内存块共享，空闲插槽链表
	Slot* _lastSlot; //当前内存块最后边界标记位置
	std::mutex _blockMutex;
};

struct PoolDeleter;

class AYMemoryPoolProxy : public Mod_MemoryPool
{
public:
	virtual void init() override;
	virtual void shutdown() override {} //不做任何操作，以达到任何时候内存池总是最后析构
	virtual void update(float delta_time) override {};
	static void initMemoryPool();
	static AYMemoryPool& getMemoryPool(uint32_t index);

	static void* useMemoryPool(size_t size);
	static void freeMomory(void* ptr, size_t size);

public:
	template<typename T, typename... Args>
	static T* createObject(Args&&... args);

	template<typename T>
	static void destroyObject(T* ptr);

public:
	template<typename T, typename... Args>
	friend T* NewObject(Args&&... args);

	template<typename T>
	friend void DeleteObject(T* ptr);

	// 智能指针和内存池是两个体系，尽量不要混用
	template<typename T, typename... Args>
	friend std::shared_ptr<T> MakeMShared(Args&&... args);

	template<typename T, typename... Args>
	friend std::unique_ptr<T, PoolDeleter> MakeMUnique(Args&&... args);
};

REGISTER_MODULE_CLASS("MemoryPool", AYMemoryPoolProxy);

struct PoolDeleter {
	template<typename T>
	void operator()(T* ptr) const {
		if (ptr) {
			static int i = 0;
			//std::string text = std::string("PoolDeleter released ") 
			//	+ typeid(T).name() 
			//	+ " :(" + std::to_string(i++) + ")\n";
			//std::cout << text;  // 日志
			AYMemoryPoolProxy::destroyObject(ptr);  // 调用内存池的销毁逻辑
		}
	}
};

inline constexpr PoolDeleter pool_deleter; // 单例删除器

template<typename T, typename... Args>
T* AYMemoryPoolProxy::createObject(Args&&... args)
{
	T* p = nullptr;
	// 根据元素大小选取合适的内存池分配内存
	if ((p = reinterpret_cast<T*>(AYMemoryPoolProxy::useMemoryPool(sizeof(T)))) != nullptr)
		// 在分配的内存上构造对象
		new(p) T(std::forward<Args>(args)...);
	return p;
}

template<typename T>
void AYMemoryPoolProxy::destroyObject(T* ptr)
{
	if (ptr)
	{
		// 调用对象的析构函数
		ptr->~T();
		// 释放内存
		freeMomory(reinterpret_cast<void*>(ptr), sizeof(T));
	}
}


template<typename T, typename... Args>
T* NewMObject(Args&&... args)
{
	return AYMemoryPoolProxy::createObject<T>(std::forward<Args>(args)...);
}

template<typename T>
void DeleteMObject(T* ptr)
{
	AYMemoryPoolProxy::destroyObject<T>(ptr);
}


template<typename T, typename ...Args>
std::shared_ptr<T> MakeMShared(Args&& ...args)
{
	T* p = AYMemoryPoolProxy::createObject<T>(std::forward<Args>(args)...);
	return std::shared_ptr<T>(p, pool_deleter);	//删除器是shard ptr对象的一部分
}

template<typename T, typename ...Args>
std::unique_ptr<T, PoolDeleter> MakeMUnique(Args&& ...args)
{
	T* p = AYMemoryPoolProxy::createObject<T>(std::forward<Args>(args)...);
	return std::unique_ptr<T, PoolDeleter>(p);		//删除器是unique ptr类型的一部分
}

