#include "AYEventSystem.h"
#include "AYEventThreadPoolManager.h"
#include "AYEventRegistry.h"
#include "IAYEvent.h"
#include "AYEventToken.h"
#include "Event_Int.h"
#include <memory>
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>



namespace AYEventSystemUnitTest
{
	std::shared_ptr<AYEventSystem> eventSystem;

	class MyClass
	{
	public:
		int a;
		float b;
		double c;
	};

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
			tokens.push_back(std::unique_ptr<AYEventToken>(
				eventSystem->subscribe("AnEvent",
					std::bind(&A::dosomthing,this,std::placeholders::_1))));
			tokens.push_back(std::unique_ptr<AYEventToken>(
				eventSystem->subscribe("MyEvent",
					std::bind(&A::listen,this,std::placeholders::_1))));
			tokens.push_back(std::unique_ptr<AYEventToken>(
				eventSystem->subscribe("EventWithInt",
					std::bind(&A::likeReduceHealth,this,std::placeholders::_1))));
			tokens.push_back(std::unique_ptr<AYEventToken>(
				eventSystem->subscribe("Event_Int",
					std::bind(&A::likeReduceHealth,this,std::placeholders::_1))));

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
			void* p = malloc(16);  // 模拟高频内存分配
			free(p);               // 模拟释放
			//std::cout << "current health is: " << health << std::endl;

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

		auto event1 = MakeMUnique<MyEvent>();
		event1->message = "benchmark_BatchHandleEvent";
		eventSystem->publish(std::move(event1));

		auto event2 = MakeMUnique<AnEvent>();
		eventSystem->publish(std::move(event2));

		eventSystem->update(0);

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_BatchHandleEvent*************************************END\n";
	}

	void benchmark_PriorityQueueHandleEvent()
	{
		std::cout << "benchmark_PriorityQueueHandleEvent*************************************BEGIN\n";

		std::shared_ptr<AYEventSystem_Mock> mockSystem = std::make_shared<AYEventSystem_Mock>();

		auto event1 = MakeMUnique<MyEvent>();
		event1->message = "benchmark_PriorityQueueHandleEvent";
		mockSystem->publish(std::move(event1));

		auto event2 = MakeMUnique<AnEvent>();
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
			AYEventRegistry::publish("Event_Int", [](IAYEvent* event) {
				auto eI = static_cast<Event_Int*>(event);
				eI->carryer = rand() % 10;
				});
		}
		eventSystem->update(0);

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_MergeEvent*************************************END\n";
	}
	void performance_EventWithMemoryPool()
	{
		std::vector<std::unique_ptr<A>> a;
		for (int i = 0; i < 5; i++)
			a.push_back(std::make_unique<A>());
		for (int i = 0; i < 100000; i++)
		{
			AYEventRegistry::publish("Event_Int", [](IAYEvent* event) {
				auto eI = static_cast<Event_Int*>(event);
				eI->carryer = rand() % 10;
				});
			if (i % 2 == 0)
				eventSystem->update(9);
		}
		for (int i = 0; i < 10000; i++)
		{
			AYEventRegistry::publish("Event_Int", [](IAYEvent* event) {
				auto eI = static_cast<Event_Int*>(event);
				eI->carryer = rand() % 10;
				});
			if (i % 1 == 0)
				eventSystem->update(0);
		}
		std::this_thread::sleep_for(std::chrono::seconds(120));
	}

	void performance_MemoryPool()
	{
		std::vector<std::unique_ptr<A>> a;
		for (int i = 0; i < 5; i++)
			a.push_back(std::make_unique<A>());
		for (int i = 0; i < 100000; i++)
		{
			std::cout << i << std::endl;
			auto x = MakeMUnique<Event_Int>();
			if (i % 2 == 0)
				eventSystem->update(9);
		}
		for (int i = 0; i < 10000; i++)
		{
			std::cout << i << std::endl;
			auto x = MakeMUnique<Event_Int>();
			if (i % 1 == 0)
				eventSystem->update(0);
		}
		std::this_thread::sleep_for(std::chrono::seconds(120));
	}
	void performance_NoMemoryPool()
	{
		std::vector<std::unique_ptr<A>> a;
		for (int i = 0; i < 1; i++)
			a.push_back(std::make_unique<A>());
		for (int i = 0; i < 100000; i++)
		{
			std::cout << i << std::endl;
			//auto x = std::make_unique<Event_Int>();
			auto x = new MyClass();
			if (i % 2 == 0)
				eventSystem->update(9);
		}
		for (int i = 0; i < 10000; i++)
		{
			std::cout << i << std::endl;
			auto x = std::make_unique<Event_Int>();
			if (i % 1 == 0)
				eventSystem->update(0);
		}
		std::this_thread::sleep_for(std::chrono::seconds(180));
	}
}

int main() {
	std::cout << "start" << std::endl;
	//auto manager = std::make_unique<AYEventSystemUnitTest::AYEventSystem_Mock>();
	//AYEventSystemUnitTest::eventSystem = std::make_shared<AYEventSystem>(std::move(manager));
	AYEventSystemUnitTest::eventSystem = GET_CAST_MODULE(AYEventSystem, "EventSystem");

	AYMemoryPoolProxy::initMemoryPool();
	GET_CAST_MODULE(AYEventSystem, "EventSystem")->init();

	//AYEventSystemUnitTest::benchmark_MultiThreadsHandleEvent1();

	//AYEventSystemUnitTest::benchmark_BatchHandleEvent();

	//AYEventSystemUnitTest::benchmark_PriorityQueueHandleEvent();

	//AYEventSystemUnitTest::benchmark_MergeEvent();

	AYEventSystemUnitTest::performance_EventWithMemoryPool();
	//AYEventSystemUnitTest::performance_MemoryPool();
	//AYEventSystemUnitTest::performance_NoMemoryPool();

	AYEventSystemUnitTest::eventSystem->~AYEventSystem();
	std::cout << "end?\n";
	getchar();
	return 0;
	//内存池一定要最后释放
}
