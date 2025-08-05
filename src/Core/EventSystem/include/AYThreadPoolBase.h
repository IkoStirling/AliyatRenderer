#pragma once
#include <functional>
#include <mutex>
#include <queue>
#include <future>
#include  <condition_variable>

// 只负责处理函数指针任务，不添加多余功能
class AYThreadPoolBase
{
public:
	using Task = std::function<void()>;
public:

	explicit AYThreadPoolBase(size_t in_threadNums);

	~AYThreadPoolBase();

	template<typename F, typename... Args>
	auto enqueue(F&& f, Args&&... args)->std::future<decltype(f(args...))>;
	
	void stop();
private:
	std::vector<std::thread> _workers;
	std::mutex _queueMutex;
	std::condition_variable _conditionVariable;
	std::queue<Task> _tasksQueue; 
	bool shouldThreadStop{ false };
};

template<typename F, typename ...Args>
inline auto AYThreadPoolBase::enqueue(F&& f, Args && ...args)->std::future<decltype(f(args ...))>
{
using ReturnType = decltype(f(args...));

	auto task = std::make_shared<std::packaged_task<ReturnType()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
	std::future<ReturnType> res = task->get_future();
	{
		std::lock_guard<std::mutex> lock(_queueMutex);
		_tasksQueue.emplace([task]() {
			(*task)();
			});
	}
	_conditionVariable.notify_one();
	return res;
}
