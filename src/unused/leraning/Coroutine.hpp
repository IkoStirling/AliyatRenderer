#pragma once
#include <iostream>
#include <functional>
#include <thread>
#include <coroutine>


namespace Coroutine {
	class IntReader
	{
	private:
		int _value{};
	public:

		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<> handle)
		{
			std::thread t([this,handle]() {
				_sleep(1000);
				_value = 1;

				handle.resume();
				});

			t.detach();
		}
		int await_resume() { return _value; }
	};

	class Task 
	{
	public:
		class promise_type
		{
		public:
			Task get_return_object() { return {}; }
			std::suspend_never initial_suspend() { return {}; }
			std::suspend_never final_suspend() noexcept{ return {}; }
			void unhandled_exception() {}
			void return_void(){}
			//void return_value(){}
		};
	};

	Task printInt()
	{
		IntReader reader1;
		int total = co_await reader1;

		IntReader reader2;
		total += co_await reader2;

		IntReader reader3;
		total += co_await reader3;

		std::cout << total << "\n";

		co_return;
	}

}