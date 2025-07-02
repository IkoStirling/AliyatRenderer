#pragma once
#include <iostream>
#include <Windows.h>
#include "AYEngineCore.h"

int main()
{
	AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();
	return 0;
}