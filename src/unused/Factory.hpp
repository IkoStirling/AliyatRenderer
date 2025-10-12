#pragma once
#include <iostream>
namespace Factory
{
	/*
		使用场景：
			实现相同接口的类，由一个创建者管理
		创建逻辑与使用逻辑分离
		产品决定使用，工厂决定创建
		工厂有权自主决定生产哪一类的产品
	*/
	enum ENUMProduct
	{
		A,
		B
	};

	class Product
	{
	public:
		virtual void use() = 0;
	};

	class ProductA : public Product
	{
	public:
		virtual void use()override
		{
			std::cout << "ProductA" << std::endl;
		}
	};
	class ProductB : public Product
	{
	public:
		virtual void use()override
		{
			std::cout << "ProductB" << std::endl;
		}
	};

	class Factory
	{
	public:
		static Product* createProduct(ENUMProduct product)
		{
			switch (product)
			{
			case A:
				return new ProductA();
				break;
			case B:
				return new ProductB();
				break;
			default:
				return nullptr;
				break;
			}
		}
	};
}