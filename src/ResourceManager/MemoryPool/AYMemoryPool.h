#pragma once
#include <atomic>
#include <mutex>

#define MAX_SLOT_SIZE 512
#define SLOT_BASE_SIZE 8
#define POOL_NUMS 64



struct Slot
{
	std::atomic<Slot*> next;
};

/*
	每个内存块首部包含slot指针，用于索引下一个内存块
	除此之外，实际存储对象的插槽，并不包含slot指针，仅由内存池主动管理（计算偏移）
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
	int _blockSize; //内存块大小
	int _slotSize; //实际插槽大小
	Slot* _firstBlock; //指向最新的内存块，链表
	Slot* _curSlot; //当前内存块标记位置，创建内存块会被更新
	std::atomic<Slot*> _freeSlots; //所有线程及内存块共享，空闲插槽链表
	Slot* _lastSlot; //当前内存块最后边界标记位置
	std::mutex _blockMutex;
};

class AYMemoryPoolProxy
{
public:
	static void initMemoryPool();
	static AYMemoryPool& getMemoryPool(int index);

	static void* useMemoryPool(size_t size);
	static void freeMomory(void* ptr, size_t size);

public:
	
public:
	template<typename T, typename... Args>
	friend T* NewObject(Args... args);

	template<typename T>
	friend void DeleteObject(T* ptr);
};

template<typename T, typename... Args>
T* NewObject(Args&&... args)
{
	T* p = nullptr;
	// 根据元素大小选取合适的内存池分配内存
	if ((p = reinterpret_cast<T*>(AYMemoryPoolProxy::useMemoryPool(sizeof(T)))) != nullptr)
		// 在分配的内存上构造对象
		new(p) T(std::forward<Args>(args)...);

	return p;
	T* ptr = reinterpret_cast<T*>(AYMemoryPoolProxy::useMemoryPool(sizeof(T)));
	if (ptr)
		new(ptr) T(std::forward<Args>(args)...);
	return ptr;
}

template<typename T>
void DeleteObject(T* ptr)
{
	if (ptr)
	{
		ptr->~T();
		AYMemoryPoolProxy::freeMomory(reinterpret_cast<void*>(ptr), sizeof(T));
	}
}

