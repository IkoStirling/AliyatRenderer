#pragma
#include <iostream>

namespace AbstractFactory
{
	/*
		使用场景：
			切换一整套产品，例如从windows切换到linux
		工厂模式的抽象，及创建者被抽象成多个，每个工厂负责生产一整套产品
	*/
	class Button
	{
	public:
		virtual void render() = 0;
		virtual void click() = 0;
	};

	class GLButton : public Button
	{
	public:
		virtual void render()override
		{
			std::cout << "GLButton" << std::endl;
		}
		virtual void click()override
		{
			std::cout << "GLButton is clicked" << std::endl;
		}
	};

	class VulkanButton : public Button
	{
	public:
		virtual void render()override
		{
			std::cout << "VulkanButton" << std::endl;
		}
	};

	class D12XButton : public Button
	{
	public:
		virtual void render()override
		{
			std::cout << "D12XButton" << std::endl;
		}
	};

	class GUIFactory
	{
	public:
		virtual Button* createButton() = 0;
	};

	class GLGUIFactory : public GUIFactory
	{
	public:
		virtual Button* createButton()override
		{
			return new  GLButton();
		}
	};

	class VulkanGUIFactory : public GUIFactory
	{
	public:
		virtual Button* createButton()override
		{
			//return new  VulkanButton();
		}
	};

	class D12XGUIFactory : public GUIFactory
	{
	public:
		virtual Button* createButton()override
		{
			//return new  D12XButton();
		}
	};

	class Test
	{
	public:
		static void run()
		{
			//std::shared_ptr<GUIFactory> f(new GLGUIFactory());
			std::shared_ptr<GUIFactory> f = std::make_shared<GLGUIFactory>();
			Button* b = f->createButton();
			b->click();
			delete b;
		}
	};
}