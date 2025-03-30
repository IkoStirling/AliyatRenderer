#pragma once
#include <functional>
#include <thread>
#include <future>
#include <queue>
#include <unordered_map>
#include <set>
#include <iostream>

// CALLBACK_WITH_CLASS_NAME : like MyClass::func
#define SUBSCRIBE_EVENT(EVENT_SYSTEM,EVENT_NAME,CALLBACK_WITH_CLASS_NAME) \
	EVENT_SYSTEM->registerEventCallback(EVENT_NAME,std::bind(&CALLBACK_WITH_CLASS_NAME,this,std::placeholders::_1))

class IAYEvent;
class AYEventThreadPool;
class AYEventToken;
class AYEventSystem;

enum class AYEventLayer :  size_t
{
	GAME_LOGIC = 0,
	PHYSICS = 1,
	RENDER = 2,
	NETWORK = 3,


	_END,
};

static const int LAYER_NUMS = static_cast<size_t>(AYEventLayer::_END);


class IAYEvent
{
public:
	class Builder;
public:
	IAYEvent(Builder builder) :
		priority(builder._priority),
		shouldMerge(builder._shouldMerge),
		layer(builder.layer)
	{}

	virtual ~IAYEvent() = default;
	virtual const char* getType()const = 0;
	virtual std::unique_ptr<IAYEvent> clone()const = 0;
	virtual void merge(const IAYEvent& other) = 0;

	const int priority;
	bool shouldMerge;
	AYEventLayer layer;

	bool operator<(const IAYEvent& other) const
	{
		return  priority < other.priority;
	}

	bool operator>(const IAYEvent& other) const
	{
		return  priority > other.priority;
	}

public:
	class Builder
	{
	public:
		Builder() {};

		Builder& setPriority(int in_priority) {
			this->_priority = in_priority;
			return *this;
		}
		Builder& setMerge(bool in_tickOnce) {
			this->_shouldMerge = in_tickOnce;
			return *this;
		}
		Builder& setLayer(AYEventLayer in_layer) {
			this->layer = in_layer;
			return *this;
		}

		int _priority{ 99 };
		bool _shouldMerge{ true }; //不合并代表一个逻辑更新期间只触发一次
		AYEventLayer layer{ AYEventLayer::GAME_LOGIC };
	};
};

class AYEventThreadPool
{
public:
	using Task = std::function<void()>;
public:

	explicit AYEventThreadPool();

	~AYEventThreadPool();

	template<typename F, typename... Args>
	auto enqueue(AYEventLayer in_layer,F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		size_t layer = static_cast<size_t>(in_layer);
		using ReturnType = decltype(f(args...));

		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);
		std::future<ReturnType> res = task->get_future();
		{
			std::lock_guard<std::mutex> lock(*_queueMutexs[layer]);
			_tasksList[layer].emplace([task]() {
				(*task)();
				});
		}
		_eventCVs[layer]->notify_one();
		//std::cout << "call threads which layer_id is: " << layer << std::endl;
		return res;
	}

private:
	std::vector<std::vector<std::thread>> _workersList;
	std::vector<std::unique_ptr<std::mutex>> _queueMutexs;
	std::vector<std::unique_ptr<std::condition_variable>> _eventCVs;
	std::vector<std::queue<Task>> _tasksList; //packaged func_ptr for callback
	bool shouldThreadStop{ false };
};

/*
* Someone who hold this instance, can still subscribe the event;
*/
class AYEventToken
{
public:
	using EventCallback = std::function<void()>;
	using EventHandler = std::function<void(const IAYEvent&)>;
public:
	AYEventToken(AYEventSystem* system, const std::string& event_name, EventHandler event_callback);
	~AYEventToken();
private:
	AYEventSystem* _eventSystem; //do not manage this ptr
	std::string _eventName;
	EventHandler _eventCallback;
};

class AYEventSystem
{
public:
	using EventCallback = std::function<void()>;
	using EventHandler = std::function<void(const IAYEvent&)>;
public:

	AYEventSystem();

	~AYEventSystem();

	//handle event asynchronously
	void update();

	void triggerEventAsync(std::unique_ptr<IAYEvent> in_event);

	//just trigger eventCallback
	void triggerEventSync(std::shared_ptr<const IAYEvent> in_event);

	AYEventToken* registerEventCallback(const std::string& event_name, EventHandler event_callback);

	void unsubscribeEventCallback(const std::string& event_name, EventHandler event_callback);

	void eventEnqueue();

private:
	std::unique_ptr<AYEventThreadPool> _threadPool;

	std::mutex _handlerMutex;
	std::unordered_map<std::string, std::vector<EventHandler>> _eventHandlers;

private:
	struct IAYEventPtrWrapper
	{
		bool operator()(const std::unique_ptr<IAYEvent>& a, const std::unique_ptr<IAYEvent>& b)
		{
			return *a > *b;
		}
	};
protected:
	std::priority_queue<
		std::unique_ptr<IAYEvent>,
		std::vector<std::unique_ptr<IAYEvent>>,
		IAYEventPtrWrapper
	> _eventQueue;

	std::mutex _queueMutex;
	std::set<std::unique_ptr<IAYEvent>> _processedEvents;
};