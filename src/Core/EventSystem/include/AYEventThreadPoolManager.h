#pragma once
#include "ECEventDependence.h"
#include <set>
#include <mutex>
#include <queue>
#include <functional>
#include <unordered_map>
#include <list>

#define MAX_CACHED_EVENTS 8192

class IAYEvent;
class AYEventToken;
class AYThreadPoolBase;
struct PoolDeleter;

/*
	事件线程池实际业务类
	以下使用情况请区分：
		1）单帧事件只触发一次 （写死保留第一次触发）
		2）单帧事件合并       （基于事件提供的merge方法）
		3）无视帧循环触发     （直接本地线程计算/提交到单例线程计算）
*/
class AYEventThreadPoolManager
{
public:
	using EventHandler = std::function<void(const IAYEvent&)>;
public:
	AYEventThreadPoolManager();
	~AYEventThreadPoolManager();

	//将事件添加进待处理集合中，等待统一执行
	void publish(std::unique_ptr<IAYEvent, PoolDeleter> in_event);

	//统一执行所有事件
	void update();
	void shutdown();

	//void executeAsync();
	void execute(std::shared_ptr<const IAYEvent> in_event);
	void executeJoin(std::unique_ptr<IAYEvent, PoolDeleter> in_event);


	AYEventToken* subscribe(const std::string& event_name, EventHandler event_callback);
	void unsubscribe(const std::string& event_name, EventHandler event_callback);

protected:
	//对待处理事件集合进行优先级队列处理
	void _enquene();

private:
	//事件仿函数
	struct IAYEventGreator
	{
		bool operator()(const std::unique_ptr<IAYEvent, PoolDeleter>& a, const std::unique_ptr<IAYEvent, PoolDeleter>& b);
	};

protected:
	std::mutex _handlerMutex;
	std::unordered_map<std::string, std::list<EventHandler>> _handlers;

	std::vector<std::unique_ptr<AYThreadPoolBase>> _layerPools;

	std::vector<std::priority_queue<
		std::unique_ptr<IAYEvent, PoolDeleter>,
		std::vector<std::unique_ptr<IAYEvent, PoolDeleter>>,
		IAYEventGreator
		>> _layerQueues;

	std::mutex _processedMutex;
	std::set<std::unique_ptr<IAYEvent, PoolDeleter>> _processedEvents;
};