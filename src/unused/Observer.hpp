#include <iostream>
#include <vector>

namespace Observer
{
	class Observer
	{
	public:
		virtual void update() = 0;
	};

	class Subject
	{
	private:
		std::vector<Observer*> observers;
	public:
		void attach(Observer* o)
		{
			observers.push_back(o);
		}
		void notify()
		{
			for (auto o : observers)
			{
				o->update();
			}
		}
	};

	class ObserverA : public Observer
	{
	public:
		virtual void update()override
		{
			std::cout << "ObserverA is Triggered" << std::endl;
		}
	};

	class Test
	{
	public:
		static void run()
		{
			Subject* sub = new Subject();
			Observer* o = new ObserverA();
			sub->attach(o);
			sub->notify();
		}
	};
}