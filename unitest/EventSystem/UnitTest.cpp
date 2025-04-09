#include "Core/EventSystem/AYEventSystem.h"
#include "Core/EventSystem/AYEventThreadPoolManager.h"
#include "Core/EventSystem/AYEventRegistry.h"
#include "Core/EventSystem/IAYEvent.h"
#include "Core/EventSystem/AYEventToken.h"
#include "Core/EventSystem/SystemEventType/EventWithInt.h"
#include "Core/core.h"
#include <memory>
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>





namespace AYEventSystemUnitTest
{
	std::shared_ptr<AYEventSystem> eventSystem;

	class AYEventSystem_Mock : public AYEventThreadPoolManager
	{
	public:
		void enque()
		{
			this->_enquene();
		}
	};

	class MyEvent :public IAYEvent
	{
		SUPPORT_MEMORY_POOL(MyEvent);
		DECLARE_EVENT_CLASS(MyEvent, "MyEvent");
	public:

		MyEvent():
			IAYEvent(
				IAYEvent::Builder()
				.setPriority(1)
				.setMerge(true)
			)
		{
		}

		virtual void merge(const IAYEvent& other) override
		{
		}

		std::string message;
	};

	REGISTER_EVENT_CLASS(MyEvent);


	class AnEvent :public IAYEvent
	{
		SUPPORT_MEMORY_POOL(AnEvent);
		DECLARE_EVENT_CLASS(AnEvent, "AnEvent");
	public:
		AnEvent() :
			IAYEvent(
				IAYEvent::Builder()
					.setPriority(99)
					.setMerge(true)
					//.setLayer(AYEventLayer::RENDER)
				)
		{
		}

		virtual void merge(const IAYEvent& other) override
		{
		}
	};

	REGISTER_EVENT_CLASS(AnEvent);

	class EventWithInt :public IAYEvent
	{
		SUPPORT_MEMORY_POOL(EventWithInt);
		DECLARE_EVENT_CLASS(EventWithInt, "EventWithInt");
	public:
		EventWithInt() :
			IAYEvent(
				IAYEvent::Builder()
				.setPriority(1)
				.setMerge(true)
			)
		{
		}
		virtual void merge(const IAYEvent& other) override
		{
			this->value += static_cast<const EventWithInt&>(other).value;
		}
	public:
		int value;
	};

	REGISTER_EVENT_CLASS(EventWithInt);

	class A
	{
	public:
		A() {
			auto token1 = SUBSCRIBE_EVENT(eventSystem, "AnEvent", A::dosomthing);
			tokens.push_back(std::unique_ptr<AYEventToken>(token1));
			auto token2 = SUBSCRIBE_EVENT(eventSystem, "MyEvent", A::listen);
			tokens.push_back(std::unique_ptr<AYEventToken>(token2));
			auto token3 = SUBSCRIBE_EVENT(eventSystem, "EventWithInt", A::likeReduceHealth);
			tokens.push_back(std::unique_ptr<AYEventToken>(token3));
			auto token4 = SUBSCRIBE_EVENT(eventSystem, "Event_Int", A::likeReduceHealth);
			tokens.push_back(std::unique_ptr<AYEventToken>(token4));
		}
		~A() {

		}
		void listen(const IAYEvent& in_event)
		{
			static int count = 0;
			auto _end = std::chrono::system_clock::now();
			std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(_end.time_since_epoch()).count() << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(0));
			std::string str = "event message is : " + static_cast<const MyEvent&>(in_event).message + " " + std::to_string(++count) +"\n";
			std::cout << str;
		}
		void dosomthing(const IAYEvent& in_event)
		{
			//这里局部静态变量是基于函数的，与实例无关
			//因此，多个观察者实例同时订阅事件会同时增长该值，最好直接使用普通成员变量或者从其他类获取多线程安全的值
			static int count = 0;
			if (++count > 3)
				return;
			if (count == 3)
			{
				std::cout << "You dosomthing third times\n";
				return;
			}
			std::cout << "You dosomthing\n";
		}

		void likeReduceHealth(const IAYEvent& in_event)
		{
			//这里局部静态变量是基于函数的，与实例无关
			//因此，多个观察者实例同时订阅事件会同时增长该值，最好直接使用普通成员变量或者从其他类获取多线程安全的值
			static int health = 100;
			health -= static_cast<const EventWithInt&>(in_event).value;
			std::cout << "current health is: " << health << std::endl;

		}
	private:
		std::vector<std::unique_ptr<AYEventToken>> tokens;
	};
	
	void benchmark_MultiThreadsHandleEvent1()
	{
		std::cout << "benchmark_MultiThreadsHandleEvent1*************************************BEGIN\n";
		std::vector<std::unique_ptr<A>> a;
		for(int i = 0; i < 5; i++)
			a.push_back(std::make_unique<A>());

		auto _end = std::chrono::system_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(_end.time_since_epoch()).count() << std::endl;

		auto event = std::make_unique<MyEvent>();
		event->message = "benchmark_MultiThreadsHandleEvent1";
		eventSystem->execute(std::move(event));

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_MultiThreadsHandleEvent1*************************************END\n";
	}

	void benchmark_BatchHandleEvent()
	{
		std::cout << "benchmark_BatchHandleEvent*************************************BEGIN\n";
		std::vector<std::unique_ptr<A>> a;
		for (int i = 0; i < 5; i++)
			a.push_back(std::make_unique<A>());

		auto event1 = std::make_unique<MyEvent>();
		event1->message = "benchmark_BatchHandleEvent";
		eventSystem->publish(std::move(event1));

		auto event2 = std::make_unique<AnEvent>();
		eventSystem->publish(std::move(event2));

		eventSystem->update();

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_BatchHandleEvent*************************************END\n";
	}

	void benchmark_PriorityQueueHandleEvent()
	{
		std::cout << "benchmark_PriorityQueueHandleEvent*************************************BEGIN\n";

		std::shared_ptr<AYEventSystem_Mock> mockSystem = std::make_shared<AYEventSystem_Mock>();

		auto event1 = std::make_unique<MyEvent>();
		event1->message = "benchmark_PriorityQueueHandleEvent";
		mockSystem->publish(std::move(event1));

		auto event2 = std::make_unique<AnEvent>();
		mockSystem->publish(std::move(event2));

		mockSystem->enque();


		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_PriorityQueueHandleEvent*************************************END\n";
	}
	void benchmark_MergeEvent()
	{
		std::cout << "benchmark_MergeEvent*************************************BEGIN\n";
		std::vector<std::unique_ptr<A>> a;
		for (int i = 0; i < 1; i++)
			a.push_back(std::make_unique<A>());
		srand(time(0));

		for (int i = 0; i < 5; i++)
		{
			AYEventRegistry::publish(eventSystem,"Event_Int", [](IAYEvent* event) {
				auto eI = static_cast<Event_Int*>(event);
				eI->carryer = rand() % 10;
				});
		}
		eventSystem->update();

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_MergeEvent*************************************END\n";
	}
}

int main() {
	std::cout << "start" << std::endl;
	auto manager = std::make_unique<AYEventSystemUnitTest::AYEventSystem_Mock>();
	AYEventSystemUnitTest::eventSystem = std::make_shared<AYEventSystem>(std::move(manager));

	AYMemoryPoolProxy::initMemoryPool();
	{
		std::vector<std::unique_ptr<AYEventSystemUnitTest::A>> a;
		for (int i = 0; i < 5; i++)
			a.push_back(std::make_unique<AYEventSystemUnitTest::A>());
		for (int i = 0; i < 100000; i++)
		{
			DEBUG_COLLECT();
			AYEventRegistry::publish(AYEventSystemUnitTest::eventSystem, "Event_Int", [](IAYEvent* event) {
				auto eI = static_cast<Event_Int*>(event);
				eI->carryer = rand() % 10;
				});
			if(i%2==0)
				AYEventSystemUnitTest::eventSystem->update();
		}
		for (int i = 0; i < 10000; i++)
		{
			DEBUG_COLLECT();
			AYEventRegistry::publish(AYEventSystemUnitTest::eventSystem, "Event_Int", [](IAYEvent* event) {
				auto eI = static_cast<Event_Int*>(event);
				eI->carryer = rand() % 10;
				});
			if (i % 1 == 0)
				AYEventSystemUnitTest::eventSystem->update();
		}
	}
	

	//AYEventSystemUnitTest::benchmark_MultiThreadsHandleEvent1();

	//AYEventSystemUnitTest::benchmark_BatchHandleEvent();

	//AYEventSystemUnitTest::benchmark_PriorityQueueHandleEvent();

	//AYEventSystemUnitTest::benchmark_MergeEvent();
	std::this_thread::sleep_for(std::chrono::seconds(180));
	DEBUG_CONSOLE_SHOW(main);

	AYEventSystemUnitTest::eventSystem->~AYEventSystem();
	std::cout << "end?\n";
	//getchar();
	return 0;
	//内存池一定要最后释放
}

/*
	事件注册支持带参构造
	增加带参宏来声明构造函数 + 函数闭包 
	带参构造和clone可以用宏一起声明
*/