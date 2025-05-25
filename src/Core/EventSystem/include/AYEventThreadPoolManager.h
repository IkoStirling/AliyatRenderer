#pragma once

#include <set>
#include <mutex>
#include <queue>
#include <functional>
#include <unordered_map>
#include <list>
#include "ECEventDependence.h"

class IAYEvent;
class AYEventToken;
class AYThreadPoolBase;

/*
	�¼��̳߳�ʵ��ҵ����
	����ʹ����������֣�
		1����֡�¼�ֻ����һ�� ��д��������һ�δ�����
		2����֡�¼��ϲ�       �������¼��ṩ��merge������
		3������֡ѭ������     ��ֱ�ӱ����̼߳���/�ύ�������̼߳��㣩
*/
class AYEventThreadPoolManager
{
public:
	using EventHandler = std::function<void(const IAYEvent&)>;
public:
	AYEventThreadPoolManager();
	~AYEventThreadPoolManager();

	//���¼���ӽ����������У��ȴ�ͳһִ��
	void publish(std::unique_ptr<IAYEvent> in_event);

	//ͳһִ�������¼�
	void update();

	//void executeAsync();
	void execute(std::shared_ptr<const IAYEvent> in_event);
	void executeJoin(std::unique_ptr<IAYEvent> in_event);


	AYEventToken* subscribe(const std::string& event_name, EventHandler event_callback);
	void unsubscribe(const std::string& event_name, EventHandler event_callback);

protected:
	//�Դ������¼����Ͻ������ȼ����д���
	void _enquene();

private:
	//�¼��º���
	struct IAYEventGreator
	{
		bool operator()(const std::unique_ptr<IAYEvent>& a, const std::unique_ptr<IAYEvent>& b);
	};

protected:
	std::mutex _handlerMutex;
	std::unordered_map<std::string, std::list<EventHandler>> _handlers;

	std::vector<std::unique_ptr<AYThreadPoolBase>> _layerPools;

	std::vector<std::priority_queue<
		std::unique_ptr<IAYEvent>,
		std::vector<std::unique_ptr<IAYEvent>>,
		IAYEventGreator
		>> _layerQueues;

	std::mutex _processedMutex;
	std::set<std::unique_ptr<IAYEvent>> _processedEvents;
};