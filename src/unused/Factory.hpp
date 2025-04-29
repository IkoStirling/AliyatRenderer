#pragma once
#include <iostream>
namespace Factory
{
	/*
		ʹ�ó�����
			ʵ����ͬ�ӿڵ��࣬��һ�������߹���
		�����߼���ʹ���߼�����
		��Ʒ����ʹ�ã�������������
		������Ȩ��������������һ��Ĳ�Ʒ
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