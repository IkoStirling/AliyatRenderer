#include "AYEngineCore.h"
#include "AYTimerManager.h"
#include <iostream>

int main()
{
	std::cout << "now begin;\r\n";
	
	AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();

	auto x = getchar();
	return 0;
}