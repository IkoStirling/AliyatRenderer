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
	ÿ���ڴ���ײ�����slotָ�룬����������һ���ڴ��
	����֮�⣬ʵ�ʴ洢����Ĳ�ۣ���������slotָ�룬�����ڴ��������������ƫ�ƣ�
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
	int _blockSize; //�ڴ���С
	int _slotSize; //ʵ�ʲ�۴�С
	Slot* _firstBlock; //ָ�����µ��ڴ�飬����
	Slot* _curSlot; //��ǰ�ڴ����λ�ã������ڴ��ᱻ����
	std::atomic<Slot*> _freeSlots; //�����̼߳��ڴ�鹲�����в������
	Slot* _lastSlot; //��ǰ�ڴ�����߽���λ��
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
	// ����Ԫ�ش�Сѡȡ���ʵ��ڴ�ط����ڴ�
	if ((p = reinterpret_cast<T*>(AYMemoryPoolProxy::useMemoryPool(sizeof(T)))) != nullptr)
		// �ڷ�����ڴ��Ϲ������
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

