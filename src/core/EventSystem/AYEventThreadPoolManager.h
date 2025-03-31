#pragma once
#include "ECEventDependence.h"
#include "IAYEventSystem.h"
#include <set>
#include <mutex>
#include <queue>
#include <unordered_map>

class IAYEvent;
class AYEventToken;
class AYThreadPoolBase;

/*
	事件线程池实际业务类
	以下使用情况请区分：
		1）单帧事件只触发一次 （写死保留第一次触发）
		2）单帧事件合并       （基于事件提供的merge方法）
		3）无视帧循环触发     （直接本地线程计算/提交到单例线程计算）
*/
class AYEventThreadPoolManager : public IAYEventSystem
{
public:
	AYEventThreadPoolManager();
	~AYEventThreadPoolManager();

	//将事件添加进待处理集合中，等待统一执行
	void publish(std::unique_ptr<IAYEvent> in_event) override;

	//统一执行所有事件
	void update() override;

	//void executeAsync();
	void execute(std::shared_ptr<const IAYEvent> in_event) override;
	void executeNow(std::unique_ptr<IAYEvent> in_event);


	AYEventToken* subscribe(const std::string& event_name, EventHandler event_callback) override;
	void unsubscribe(const std::string& event_name, EventHandler event_callback) override;

private:
	//对待处理事件集合进行优先级队列处理
	void _enquene();

private:
	struct IAYEventGreator
	{
		bool operator()(const std::unique_ptr<IAYEvent>& a, const std::unique_ptr<IAYEvent>& b);
	};

private:
	std::mutex _handlerMutex;
	std::unordered_map<std::string, std::vector<EventHandler>> _handlers;

	std::vector<std::unique_ptr<AYThreadPoolBase>> _layerPools;

	std::vector<std::priority_queue<
		std::unique_ptr<IAYEvent>,
		std::vector<std::unique_ptr<IAYEvent>>,
		IAYEventGreator
		>> _layerQueues;

	std::mutex _processedMutex;
	std::set<std::unique_ptr<IAYEvent>> _processedEvents;
};