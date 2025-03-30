#include "AYEventSystem.h"
#include <iostream>

AYEventThreadPool::AYEventThreadPool()
{
	//提前分配vector大小
	//从配置文件中读取线程数量期望值
	size_t maxNumThreads = std::thread::hardware_concurrency();
	size_t expectGameLogic = 2;
	size_t expectPhysics = 1;
	size_t expectRender = 1;
	size_t expectNetwork = 1;
	std::vector<size_t> expectNumThreads;
	expectNumThreads.push_back(expectGameLogic);
	expectNumThreads.push_back(expectPhysics);
	expectNumThreads.push_back(expectRender);
	expectNumThreads.push_back(expectNetwork);
	
	_workersList.reserve(LAYER_NUMS);
	for (size_t i = 0; i < expectNumThreads.size(); i++)
	{
		_workersList.emplace_back();
		_workersList[i].reserve(expectNumThreads[i]);

		_queueMutexs.push_back(std::make_unique<std::mutex>());
		_eventCVs.push_back(std::make_unique<std::condition_variable>());
		_tasksList.push_back(std::queue<Task>());
	}
	for (size_t layer = 0; layer < expectNumThreads.size(); layer++)
	{
		for (size_t j = 0; j < expectNumThreads[layer]; j++)
		{
			_workersList[layer].emplace_back([this, layer]() {
				while (true)
				{
					Task task;
					{
						std::unique_lock<std::mutex> lock(*_queueMutexs[layer]);
						_eventCVs[layer]->wait(lock, [this, layer]() {
							//线程被通知停止，但还有任务没执行完，则继续运行无需等待
							//predicate为true时无需等待，直接唤醒
							return !_tasksList[layer].empty() || shouldThreadStop;
							});
						if (_tasksList[layer].empty() && shouldThreadStop)
							return;
						task = std::move(_tasksList[layer].front());
						_tasksList[layer].pop();
					}
					task();
				}
				});
		}
	}
}

AYEventThreadPool::~AYEventThreadPool()
{
	{
		for(size_t layer = 0; layer < _queueMutexs.size(); layer++)
			std::lock_guard<std::mutex> lock(*_queueMutexs[layer]);
		shouldThreadStop = true;
	}

	for (size_t layer = 0; layer < _eventCVs.size(); layer++)
		_eventCVs[layer]->notify_all();

	for (auto& workers : _workersList)
	{
		for (auto& worker : workers)
		{
			if (worker.joinable())
				worker.join();
		}
		workers.clear();
	}
	_workersList.clear();
}

AYEventSystem::AYEventSystem()
{
	_threadPool = std::make_unique<AYEventThreadPool>();
}

AYEventSystem::~AYEventSystem()
{
}

void AYEventSystem::update()
{
	eventEnqueue();

	while (!_eventQueue.empty())
	{
		auto in_event = std::move(const_cast<std::unique_ptr<IAYEvent>&>(_eventQueue.top()));
		triggerEventSync(std::move(in_event));
		_eventQueue.pop();
	}
}

void AYEventSystem::triggerEventSync(std::shared_ptr<const IAYEvent> in_event)
{
	const std::string eventType = in_event->getType();
	std::vector<EventHandler> handlersCopy;
	{
		std::lock_guard<std::mutex> lock(_handlerMutex);
		auto it = _eventHandlers.find(eventType);
		if (it != _eventHandlers.end())
			handlersCopy = it->second;
	}

	for (const auto& handler : handlersCopy)
	{
		_threadPool->enqueue(in_event->layer,[in_event, handler_ = std::move(handler)](){
			try {
				std::unique_ptr<IAYEvent> eventCopy = in_event->clone();
				handler_(*eventCopy);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Exception in event handler: " << e.what() << std::endl;
			}
		});
	}
}

void AYEventSystem::triggerEventAsync(std::unique_ptr<IAYEvent> in_event)
{
	std::lock_guard<std::mutex> lock(_queueMutex);
	for (auto&& it : _processedEvents)
	{
		if (in_event->getType() == it->getType())
		{
			if (in_event->shouldMerge)
			{
				it->merge(*in_event);
			}
			return;
		}
	}
	_processedEvents.insert(std::move(in_event));
}

AYEventToken* AYEventSystem::registerEventCallback(const std::string& event_name, EventHandler event_callback)
{
	std::lock_guard<std::mutex> lock(_handlerMutex);
	_eventHandlers[event_name].push_back(event_callback);
	return new AYEventToken(this, event_name, event_callback);
}

void AYEventSystem::unsubscribeEventCallback(const std::string& event_name, EventHandler event_callback)
{
	auto condition = [event_callback](EventHandler& func) {
		return func.target<void(*)(const EventHandler&)>() == event_callback.target<void(*)(const EventHandler&)>();
	};
	auto& callbacks = _eventHandlers[event_name];
	for (auto it = callbacks.begin();it != callbacks.end(); it++)
	{
		if (condition(*it))
		{
			callbacks.erase(it);
			if (callbacks.empty())
			{
				_eventHandlers.erase(event_name);
			}
			return;
		}
	}
}

void AYEventSystem::eventEnqueue()
{
	std::set<std::unique_ptr<IAYEvent>> processedEventsCopy;
	{
		std::lock_guard<std::mutex> lock(_queueMutex);
		processedEventsCopy = std::move(_processedEvents);
	}

	for (auto it = processedEventsCopy.begin(); it != processedEventsCopy.end();)
	{
		_eventQueue.push(std::move(const_cast<std::unique_ptr<IAYEvent>&>(*it)));
		it = processedEventsCopy.erase(it);
	}
}

AYEventToken::AYEventToken(AYEventSystem* system, const std::string& event_name, EventHandler event_callback) :
	_eventSystem(system),
	_eventName(event_name),
	_eventCallback(event_callback)
{
}

AYEventToken::~AYEventToken()
{
	_eventSystem->unsubscribeEventCallback(_eventName, _eventCallback);
}
