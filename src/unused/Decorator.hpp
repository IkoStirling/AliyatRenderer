namespace Decorator
{
	class Component
	{
	public:
		virtual void operation() = 0;
	};

	class ComponentA : public Component
	{
	public:
		virtual void operation()override
		{
			std::cout << "ComponentA operation" << std::endl;
		}
	};

	template <typename T>
	class Decorator :public T
	{
	protected:
		std::unique_ptr<T> component;
	public:
		Decorator(std::unique_ptr<T> c) :component(std::move(c))
		{
		}
		virtual void operation()override
		{
			if (!component)
				return;
			preOperation();
			component->operation();
			postOperation();
		}
		virtual void preOperation() = 0;
		virtual void postOperation() = 0;
	};

	class MyDefinationComponent :public Decorator<ComponentA>
	{
		using Decorator::Decorator;
	public:
		MyDefinationComponent(std::unique_ptr<ComponentA> c) :
			Decorator(std::move(c))
		{
		}

		virtual void preOperation()override
		{
			std::cout << "pre" << std::endl;
		}
		virtual void postOperation()override
		{
			std::cout << "post" << std::endl;
		}
	};

	class Test
	{
	public:
		static void run()
		{
			std::unique_ptr<ComponentA> com(new ComponentA());
			Component* c = new MyDefinationComponent(std::move(com));
			c->operation();
			delete c;
		}
	};
}