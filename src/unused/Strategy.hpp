namespace Strategy
{
	class Strategy
	{
	public:
		virtual void exec() = 0;
	};

	class StrategyA : public Strategy
	{
	public:
		virtual void exec()override
		{
			std::cout << "StrategyA" << std::endl;
		}
	};

	class StrategyB : public Strategy
	{
	public:
		virtual void exec()override
		{
			std::cout << "StrategyB" << std::endl;
		}
	};

	class Context
	{
	private:
		Strategy* strategy;
	public:
		Context(Strategy* s) :strategy(s)
		{
		}
		void setStrategy(Strategy* s)
		{
			strategy = s;
		}
		void execStrategy()
		{
			if(strategy)
				strategy->exec();
		}
	};

	class Test
	{
	public:
		static void run()
		{
			Context* context = new Context(new StrategyA());
			context->execStrategy();
			context->setStrategy(new StrategyB());
			context->execStrategy();
			delete context;
		}
	};
}