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
		operator delete(reinterpret_cast<void*>(it)); //不调用析构
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
		if (_curSlot > _lastSlot)
			newBlock();
		cur = _curSlot;
		_curSlot += _slotSize / sizeof(Slot); // T* ptr+2 = ptr + 2 * sizeof(T); 只加指定字节需要从除掉T的大小
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

}

size_t AYMemoryPool::paddingBlock(char*, size_t)
{
	return size_t();
}

void AYMemoryPool::pushFreeSlot(Slot* slot)
{

}

Slot* AYMemoryPool::popFreeSlot()
{
	return nullptr;
}

void AYMemoryPoolProxay::initMemoryPool()
{
	for (int i = 0; i < POOL_NUMS; i++)
		getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
}

AYMemoryPool& AYMemoryPoolProxay::getMemoryPool(int index)
{
	static AYMemoryPool _memoryPoolList[POOL_NUMS];
	return _memoryPoolList[index];
}

void* AYMemoryPoolProxay::useMemoryPool(size_t size)
{
	if (size <= 0)
		return nullptr;
	if (size >= MAX_SLOT_SIZE)
		return operator new(size); //使用原版new

	return getMemoryPool((size / SLOT_BASE_SIZE)).allocate();
}

void AYMemoryPoolProxay::freeMomory(void* ptr, size_t size)
{
	if (!ptr)
		return;
	if (size > MAX_SLOT_SIZE)
		return operator delete(ptr);

	return getMemoryPool((size / SLOT_BASE_SIZE)).deallocate(ptr);
}
