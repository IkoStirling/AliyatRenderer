#include "AYMemoryPool.h"
#include <assert.h>

AYMemoryPool::AYMemoryPool(size_t in_blockSize) :
	_blockSize(in_blockSize),
	_slotSize(0),
	_firstBlock(nullptr),
	_curSlot(nullptr),
	_freeSlots(nullptr),
	_lastSlot(nullptr)
{
}

AYMemoryPool::~AYMemoryPool()
{
	Slot* it = _firstBlock;
	while (it)
	{
		Slot* next = it->next;
		operator delete(reinterpret_cast<void*>(it)); //����������
		it = next;
	}
}

void AYMemoryPool::init(size_t in_slotSize)
{
	assert(in_slotSize > 0);
	_slotSize = in_slotSize;
}

void* AYMemoryPool::allocate()
{
	Slot* cur = popFreeSlot();
	if (cur)
		return cur;

	{
		std::lock_guard<std::mutex> lock(_blockMutex);
		if (_curSlot >= _lastSlot)
			newBlock();
		cur = _curSlot;
		_curSlot += _slotSize / sizeof(Slot); // T* ptr+2 = ptr + 2 * sizeof(T); ֻ��ָ���ֽ���Ҫ�ӳ���T�Ĵ�С
	}

	return cur;
}

void AYMemoryPool::deallocate(void* ptr)
{
	if (!ptr)
		return;
	Slot* slot = reinterpret_cast<Slot*>(ptr);
	pushFreeSlot(slot);
}

void AYMemoryPool::newBlock()
{
	void* newBlock = operator new(_blockSize);
	reinterpret_cast<Slot*>(newBlock)->next = _firstBlock;
	_firstBlock = reinterpret_cast<Slot*>(newBlock);

	//ÿ���ڴ��ǰ�˶���һ���ڴ����ڴ洢������Ϣ�����ʵ���ڴ�λ����Ҫƫ��
	char* content = reinterpret_cast<char*>(newBlock) + sizeof(Slot);
	size_t padding = paddingBlock(content, _slotSize);

	_curSlot = reinterpret_cast<Slot*>(content + padding);
	_lastSlot = reinterpret_cast<Slot*>(reinterpret_cast<size_t>(newBlock) + _blockSize - _slotSize + 1);
	_freeSlots = nullptr;
}

size_t AYMemoryPool::paddingBlock(char* ptr, size_t align)
{
	//�����ڴ���룬cast��ͬ�ڻ�ȡָ�����ֵ��ַ
	return (align - reinterpret_cast<size_t>(ptr)) % align;
}

bool AYMemoryPool::pushFreeSlot(Slot* slot)
{
	while (true)
	{
		// ��ȡ��ǰͷ�ڵ�
		Slot* oldHead = _freeSlots.load(std::memory_order_relaxed);
		// ���½ڵ�� next ָ��ǰͷ�ڵ�
		slot->next.store(oldHead, std::memory_order_relaxed);

		// ���Խ��½ڵ�����Ϊͷ�ڵ�
		if (_freeSlots.compare_exchange_weak(oldHead, slot,
			std::memory_order_release, std::memory_order_relaxed))
		{
			return true;
		}
		// ʧ�ܣ�˵����һ���߳̿����Ѿ��޸��� freeList_
		// CAS ʧ��������
	}
}

Slot* AYMemoryPool::popFreeSlot()
{
	while (true)
	{
		Slot* oldHead = _freeSlots.load(std::memory_order_acquire);
		if (oldHead == nullptr)
			return nullptr; 

		Slot* newHead = nullptr;
		try
		{
			newHead = oldHead->next.load(std::memory_order_relaxed);
		}
		catch (...)
		{
			static int retryTimes = 0;
			if (retryTimes++ < 3)
				continue;
			else
				return nullptr;
		}

		if (_freeSlots.compare_exchange_weak(oldHead, newHead,
			std::memory_order_acquire, std::memory_order_relaxed))
		{
			return oldHead;
		}
		// ʧ�ܣ�˵����һ���߳̿����Ѿ��޸��� freeList_
		// CAS ʧ��������
	}
}

void AYMemoryPoolProxy::initMemoryPool()
{
	for (int i = 0; i < POOL_NUMS; i++)
		getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
}

AYMemoryPool& AYMemoryPoolProxy::getMemoryPool(int index)
{
	static AYMemoryPool _memoryPoolList[POOL_NUMS];
	return _memoryPoolList[index];
}

void* AYMemoryPoolProxy::useMemoryPool(size_t size)
{
	if (size <= 0)
		return nullptr;
	if (size >= MAX_SLOT_SIZE)
		return operator new(size); //ʹ��ԭ��new

	return getMemoryPool(((size + 7)/ SLOT_BASE_SIZE) -1).allocate();
}

void AYMemoryPoolProxy::freeMomory(void* ptr, size_t size)
{
	if (!ptr)
		return;
	if (size > MAX_SLOT_SIZE)
		return operator delete(ptr);

	return getMemoryPool(((size - 1) / SLOT_BASE_SIZE)).deallocate(ptr);
}
