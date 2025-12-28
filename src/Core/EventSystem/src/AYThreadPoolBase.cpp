#include "AYThreadPoolBase.h"

namespace ayt::engine::pool::thread
{
	ThreadPoolBase::ThreadPoolBase(size_t in_threadNums)
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
							//线程被通知停止，但还有任务没执行完，则继续运行无需等待
							//predicate为true时无需等待，直接唤醒
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

	ThreadPoolBase::~ThreadPoolBase()
	{
		stop();
	}

	void ThreadPoolBase::stop()
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
}