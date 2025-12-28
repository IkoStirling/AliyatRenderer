#include "AYMemoryPool.h"
#include <assert.h>
namespace ayt::engine::pool::memory
{
	MemoryPoolBase::MemoryPoolBase(size_t in_blockSize) :
		_blockSize(in_blockSize),
		_slotSize(0),
		_firstBlock(nullptr),
		_curSlot(nullptr),
		_freeSlots(nullptr),
		_lastSlot(nullptr)
	{
	}

	MemoryPoolBase::~MemoryPoolBase()
	{
		Slot* it = _firstBlock;
		while (it)
		{
			Slot* next = it->next;
			operator delete(reinterpret_cast<void*>(it)); //不调用析构
			it = next;
		}
	}

	void MemoryPoolBase::init(size_t in_slotSize)
	{
		assert(in_slotSize > 0);
		_slotSize = in_slotSize;
	}

	void* MemoryPoolBase::allocate()
	{
		Slot* cur = popFreeSlot();
		if (cur)
			return cur;

		{
			std::lock_guard<std::mutex> lock(_blockMutex);
			if (_curSlot >= _lastSlot)
				newBlock();
			cur = _curSlot;
			_curSlot += _slotSize / sizeof(Slot); // T* ptr+2 = ptr + 2 * sizeof(T); 只加指定字节需要从除掉T的大小
		}

		return cur;
	}

	void MemoryPoolBase::deallocate(void* ptr)
	{
		if (!ptr)
			return;
		Slot* slot = reinterpret_cast<Slot*>(ptr);
		pushFreeSlot(slot);
	}

	void MemoryPoolBase::newBlock()
	{
		void* newBlock = operator new(_blockSize);
		reinterpret_cast<Slot*>(newBlock)->next = _firstBlock;
		_firstBlock = reinterpret_cast<Slot*>(newBlock);

		//每个内存块前端都有一段内存用在存储索引信息，因此实际内存位置需要偏移
		char* content = reinterpret_cast<char*>(newBlock) + sizeof(Slot);
		size_t padding = paddingBlock(content, _slotSize);

		_curSlot = reinterpret_cast<Slot*>(content + padding);
		_lastSlot = reinterpret_cast<Slot*>(reinterpret_cast<size_t>(newBlock) + _blockSize - _slotSize + 1);
		_freeSlots = nullptr;
	}

	size_t MemoryPoolBase::paddingBlock(char* ptr, size_t align)
	{
		//计算内存对齐，cast等同于获取指针的数值地址
		return (align - reinterpret_cast<size_t>(ptr)) % align;
	}

	bool MemoryPoolBase::pushFreeSlot(Slot* slot)
	{
		while (true)
		{
			// 获取当前头节点
			Slot* oldHead = _freeSlots.load(std::memory_order_relaxed);
			// 将新节点的 next 指向当前头节点
			slot->next.store(oldHead, std::memory_order_relaxed);

			// 尝试将新节点设置为头节点
			if (_freeSlots.compare_exchange_weak(oldHead, slot,
				std::memory_order_release, std::memory_order_relaxed))
			{
				return true;
			}
			// 失败：说明另一个线程可能已经修改了 freeList_
			// CAS 失败则重试
		}
	}

	Slot* MemoryPoolBase::popFreeSlot()
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
			// 失败：说明另一个线程可能已经修改了 freeList_
			// CAS 失败则重试
		}
	}

	void MemoryPoolBaseProxy::init()
	{
		for (int i = 0; i < POOL_NUMS; i++)
			getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
	}

	void MemoryPoolBaseProxy::initMemoryPool()
	{
		for (int i = 0; i < POOL_NUMS; i++)
			getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
	}

	MemoryPoolBase& MemoryPoolBaseProxy::getMemoryPool(uint32_t index)
	{
		static MemoryPoolBase _memoryPoolList[POOL_NUMS];
		return _memoryPoolList[index];
	}

	void* MemoryPoolBaseProxy::useMemoryPool(size_t size)
	{
		if (size <= 0)
			return nullptr;
		if (size >= MAX_SLOT_SIZE)
			return operator new(size); //使用原版new

		return getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).allocate();
	}

	void MemoryPoolBaseProxy::freeMomory(void* ptr, size_t size)
	{
		if (!ptr)
			return;
		if (size > MAX_SLOT_SIZE)
			return operator delete(ptr);

		return getMemoryPool(((size - 1) / SLOT_BASE_SIZE)).deallocate(ptr);
	}
}