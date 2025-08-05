#include "AYEventThreadPoolManager.h"
#include "AYThreadPoolBase.h"
#include "IAYEvent.h"
#include "AYEventToken.h"
#include <iostream>

AYEventThreadPoolManager::AYEventThreadPoolManager()
{
	size_t maxNumThreads = std::thread::hardware_concurrency();
	size_t expectGameLogic = 2;
	size_t expectPhysics = 1;
	size_t expectRender = 1;
	size_t expectNetwork = 1;
	size_t expectResource = 2;
	std::vector<size_t> expectNumThreads;
	expectNumThreads.push_back(expectGameLogic);
	expectNumThreads.push_back(expectPhysics);
	expectNumThreads.push_back(expectRender);
	expectNumThreads.push_back(expectNetwork);
	expectNumThreads.push_back(expectResource);

	for (auto it : expectNumThreads)
	{
		_layerPools.push_back(std::make_unique<AYThreadPoolBase>(it));
	}
	_layerQueues.resize(LAYER_NUMS);
}

AYEventThreadPoolManager::~AYEventThreadPoolManager()
{
}

void AYEventThreadPoolManager::publish(std::unique_ptr<IAYEvent, PoolDeleter> in_event)
{
	std::lock_guard<std::mutex> lock(_processedMutex);
	if (_processedEvents.size() > MAX_CACHED_EVENTS) {
		_processedEvents.erase(_processedEvents.begin());  // 丢弃最旧事件
	}
	for (auto&& it : _processedEvents)
	{
		if (in_event->getType() == it->getType())
		{
			if (in_event->shouldMerge)
			{
				it->merge(*in_event);
				return;
			}
		}
	}
	_processedEvents.insert(std::move(in_event));
}

void AYEventThreadPoolManager::update()
{
	_enquene();

	for (size_t layer = 0; layer < LAYER_NUMS; layer++)
	{
		while (!_layerQueues[layer].empty())
		{
			auto oneEvent = std::move(const_cast<std::unique_ptr<IAYEvent, PoolDeleter>&>(_layerQueues[layer].top()));
			execute(std::move(oneEvent));
			_layerQueues[layer].pop();
		}
	}
}

void AYEventThreadPoolManager::shutdown()
{
	std::lock_guard<std::mutex> lock(_processedMutex);
	for (auto it = _processedEvents.begin(); it != _processedEvents.end();)
	{
		it = _processedEvents.erase(it);
	}

	std::vector<std::priority_queue<
		std::unique_ptr<IAYEvent, PoolDeleter>,
		std::vector<std::unique_ptr<IAYEvent, PoolDeleter>>,
		IAYEventGreator
		>> lq;
	lq.swap(_layerQueues);

	for (auto& pool : _layerPools)
	{
		pool.get()->stop();
	}
}

void AYEventThreadPoolManager::execute(std::shared_ptr<const IAYEvent> in_event)
{
	const std::string eventType = in_event->getType();
	size_t layer = static_cast<size_t>(in_event->layer);

	std::list<EventHandler> handlersCopy;
	{
		std::lock_guard<std::mutex> lock(_handlerMutex);
		auto it = _handlers.find(eventType);
		if (it != _handlers.end())
			handlersCopy = it->second;
	}

	for (const auto& handler : handlersCopy)
	{
		_layerPools[layer]->enqueue([in_event, handler_ = std::move(handler)](){
			try {
				handler_(*in_event);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Exception in event handler: " << e.what() << std::endl;
			}
		});
	}

}

void AYEventThreadPoolManager::executeJoin(std::unique_ptr<IAYEvent, PoolDeleter> in_event)
{
	const std::string eventType = in_event->getType();
	std::list<EventHandler> handlersCopy;
	{
		std::lock_guard<std::mutex> lock(_handlerMutex);
		auto it = _handlers.find(eventType);
		if (it != _handlers.end())
			handlersCopy = it->second;
	}

	for (const auto& handler : handlersCopy)
	{
		try {
			handler(*in_event);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Exception in event handler: " << e.what() << std::endl;
		}
	}
}


AYEventToken* AYEventThreadPoolManager::subscribe(const std::string& event_name, EventHandler event_callback)
{
	std::lock_guard<std::mutex> lock(_handlerMutex);
	_handlers[event_name].push_back(event_callback);
	return nullptr;
}

void AYEventThreadPoolManager::unsubscribe(const std::string& event_name, EventHandler event_callback)
{
	auto condition = [event_callback](EventHandler& func) {
		return func.target<void(*)(const EventHandler&)>() == event_callback.target<void(*)(const EventHandler&)>();
	};
	auto& callbacks = _handlers[event_name];
	for (auto it = callbacks.begin(); it != callbacks.end(); it++)
	{
		if (condition(*it))
		{
			callbacks.erase(it);
			if (callbacks.empty())
			{
				_handlers.erase(event_name);
			}
			return;
		}
	}
}


void AYEventThreadPoolManager::_enquene()
{
	std::set<std::unique_ptr<IAYEvent, PoolDeleter>> processedEventsCopy;
	{
		std::lock_guard<std::mutex> lock(_processedMutex);
		processedEventsCopy = std::move(_processedEvents);
	}

	for (auto it = processedEventsCopy.begin(); it != processedEventsCopy.end();)
	{
		size_t layer = static_cast<size_t>(it->get()->layer);
		_layerQueues[layer].push(std::move(const_cast<std::unique_ptr<IAYEvent, PoolDeleter>&>(*it)));
		it = processedEventsCopy.erase(it);
	}
}

bool AYEventThreadPoolManager::IAYEventGreator::operator()(
	const std::unique_ptr<IAYEvent, PoolDeleter>& a,
	const std::unique_ptr<IAYEvent, PoolDeleter>& b)
{
	return a->priority < b->priority;
}
