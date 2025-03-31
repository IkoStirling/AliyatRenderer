#pragma once
#include <atomic>
#include <mutex>

#define MAX_SLOT_SIZE 512
#define SLOT_BASE_SIZE 8
#define POOL_NUMS 64


/*
	通过该方式生成的任意对象首部嵌入了一个指向下一个内存插槽的指针
*/
struct Slot
{
	std::atomic<Slot*> next;
};


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
	size_t paddingBlock(char*, size_t);

	void pushFreeSlot(Slot* slot);
	Slot* popFreeSlot();
private:
	int _blockSize;
	int _slotSize;
	Slot* _firstBlock;
	Slot* _curSlot;
	std::atomic<Slot*> _freeSlots;
	Slot* _lastSlot;
	std::mutex _blockMutex;
};

class AYMemoryPoolProxay
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
T* NewObject(Args... args)
{
	T* ptr = reinterpret_cast<T*>(AYMemoryPoolProxay::useMemoryPool(sizeof(T)));
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
		AYMemoryPoolProxay::freeMomory(ptr, sizeof(T));
	}
}