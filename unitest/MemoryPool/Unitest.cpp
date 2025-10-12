#include <iostream>
#include "AYMemoryPool.h"

class MyClass {
public:
	SUPPORT_MEMORY_POOL(MyClass)
public:
	MyClass(int a, float b): valueA(a),valueB(b) { printf("MyClass constructor called with a=%d, b=%f\n", a, b); }
	// ...
	int valueA;
	float valueB;
};
class MyClass2 : public MyClass {
public:
	MyClass2(int a, float b): MyClass(a,b){ printf("MyClass constructor called with a=%d, b=%f\n", a, b); }
	// ...
};

int main()
{
	GET_CAST_MODULE(Mod_MemoryPool, "MemoryPool")->init();
	std::cout << sizeof(MyClass) << std::endl;
	
	{
		auto obj = new MyClass2(1,123.2);
		delete obj;
	}
	return 0;
}