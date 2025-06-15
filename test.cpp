#pragma once
#include <iostream>
#include "AYEngineCore.h"
#include "AYRendererManager.h"

int main()
{
	AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();

	//AYRendererManager rm;
	//rm.init();
	//rm.update(1);
	getchar();
	return 0;
}