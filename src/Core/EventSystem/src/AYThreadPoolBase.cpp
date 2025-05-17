#include "AYThreadPoolBase.h"

AYThreadPoolBase::AYThreadPoolBase(size_t in_threadNums)
{
	_workers.reserve(in_threadNums);
	
	for (size_t i = 0; i < in_threadNums; i++)
	{
		_workers.emplace_back([this]() {
			while (true)
			{
				Task task;
				{
					std::unique_lock<std::mutex> lock(_queueMutex);
					_conditionVariable.wait(lock, [this]() {
						//�̱߳�ֹ֪ͨͣ������������ûִ���꣬�������������ȴ�
						//predicateΪtrueʱ����ȴ���ֱ�ӻ���
						return !_tasksQueue.empty() || shouldThreadStop;
						});
					if (_tasksQueue.empty() && shouldThreadStop)
						return;
					task = std::move(_tasksQueue.front());
					_tasksQueue.pop();
				}
				task();
			}
		});
	}
}

AYThreadPoolBase::~AYThreadPoolBase()
{
	{
		std::lock_guard<std::mutex> lock(_queueMutex);
		shouldThreadStop = true;
	}

	_conditionVariable.notify_all();

	for (auto& worker : _workers)
	{
		if (worker.joinable())
			worker.join();
	}
	_workers.clear();
}
