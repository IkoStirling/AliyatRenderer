#pragma once
#include <iostream>

namespace Adapter
{
	class Target
	{
	public:
		virtual void request() = 0;
	};
	class Adaptee
	{
	public:
		void specificRequest()
		{
			std::cout << "specificRequest" << std::endl;
		}
	};
	class Adapter : public Target
	{
	private:
		Adaptee* adaptee;
	public:
		Adapter(Adaptee* a) :adaptee(a)
		{

		}
		virtual void request()override
		{
			adaptee->specificRequest();
		}
	};

	class Test
	{
	public:
		static void run()
		{
			std::unique_ptr<Adaptee> ptee(new Adaptee());
			std::unique_ptr<Adapter> pter(new Adapter(ptee.get()));
			pter->request();
		}
	};

}