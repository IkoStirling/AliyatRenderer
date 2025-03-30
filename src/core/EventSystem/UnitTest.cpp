#include "AYEventSystem.h"
#include <iostream>
#include <string>
#include <random>

namespace AYEventSystemUnitTest
{
	std::shared_ptr<AYEventSystem> eventSystem;
	class AYEventSystem_Mock : public AYEventSystem
	{
	public:
		const char* getPriorityQueueFront()
		{
			const char* eventType = _eventQueue.top()->getType();
			_eventQueue.pop();
			return eventType;
		}

	};

	class MyEvent :public IAYEvent
	{
	public:

		virtual const char* getType() const override
		{
			return "MyEvent";
		}

		MyEvent(const std::string& in_message) :
			message(in_message),
			IAYEvent(
				IAYEvent::Builder()
				.setPriority(99)
				.setMerge(true)
			)
		{

		}

		virtual std::unique_ptr<IAYEvent> clone()const override
		{
			return std::make_unique<MyEvent>(message);
		}

		virtual void merge(const IAYEvent& other) override
		{

		}

		std::string message;
	};

	class AnEvent :public IAYEvent
	{
	public:
		virtual const char* getType() const override
		{
			return "AnEvent";
		}
		AnEvent() :
			IAYEvent(
				IAYEvent::Builder()
					.setPriority(1)
					.setMerge(true)
				)
		{

		}
		virtual std::unique_ptr<IAYEvent> clone()const override
		{
			return std::make_unique<AnEvent>();
		}

		virtual void merge(const IAYEvent& other) override
		{

		}
	};

	class EventWithInt :public IAYEvent
	{
	public:
		virtual const char* getType() const override
		{
			return "EventWithInt";
		}
		EventWithInt(int in_value) :
			value(in_value),
			IAYEvent(
				IAYEvent::Builder()
				.setPriority(1)
				.setMerge(true)
			)
		{

		}
		virtual std::unique_ptr<IAYEvent> clone()const override
		{
			return std::make_unique<EventWithInt>(value);
		}

		virtual void merge(const IAYEvent& other) override
		{
			this->value += static_cast<const EventWithInt&>(other).value;
		}
	public:
		int value;
	};

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
		}
		~A() {

		}
		void listen(const IAYEvent& in_event)
		{
			static int count = 0;
			auto _end = std::chrono::system_clock::now();
			std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(_end.time_since_epoch()).count() << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(2));
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

		auto event = std::make_unique<MyEvent>("benchmark_MultiThreadsHandleEvent1");
		eventSystem->triggerEventSync(std::move(event));

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_MultiThreadsHandleEvent1*************************************END\n";
	}

	void benchmark_BatchHandleEvent()
	{
		std::cout << "benchmark_BatchHandleEvent*************************************BEGIN\n";
		std::vector<std::unique_ptr<A>> a;
		for (int i = 0; i < 5; i++)
			a.push_back(std::make_unique<A>());

		auto event1 = std::make_unique<MyEvent>("benchmark_BatchHandleEvent");
		eventSystem->triggerEventAsync(std::move(event1));

		auto event2 = std::make_unique<AnEvent>();
		eventSystem->triggerEventAsync(std::move(event2));

		eventSystem->update();

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_BatchHandleEvent*************************************END\n";
	}

	void benchmark_PriorityQueueHandleEvent()
	{
		std::cout << "benchmark_PriorityQueueHandleEvent*************************************BEGIN\n";

		std::shared_ptr<AYEventSystem_Mock> mockSystem = std::make_shared<AYEventSystem_Mock>();

		auto event1 = std::make_unique<MyEvent>("benchmark_PriorityQueueHandleEvent");
		mockSystem->triggerEventAsync(std::move(event1));

		auto event2 = std::make_unique<AnEvent>();
		mockSystem->triggerEventAsync(std::move(event2));

		mockSystem->eventEnqueue();
		std::cout << mockSystem->getPriorityQueueFront() << std::endl;
		std::cout << mockSystem->getPriorityQueueFront() << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_PriorityQueueHandleEvent*************************************END\n";
	}
	void benchmark_MergeEvent()
	{
		std::cout << "benchmark_MergeEvent*************************************BEGIN\n";

		auto a = std::make_unique<A>();

		for (int i = 0; i < 5; i++)
		{
			srand(time(0));
			auto event = std::make_unique<EventWithInt>(rand() % 10);
			eventSystem->triggerEventAsync(std::move(event));
		}
		eventSystem->update();

		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "benchmark_MergeEvent*************************************END\n";
	}
}

int main() {
	AYEventSystemUnitTest::eventSystem = std::make_shared<AYEventSystem>();

	//AYEventSystemUnitTest::benchmark_MultiThreadsHandleEvent1();

	//AYEventSystemUnitTest::benchmark_BatchHandleEvent();

	//AYEventSystemUnitTest::benchmark_PriorityQueueHandleEvent();

	AYEventSystemUnitTest::benchmark_MergeEvent();

	getchar();
	return 0;

}