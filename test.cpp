#pragma once
#include <iostream>
#include "AYEngineCore.h"
#include "AYRendererManager.h"
#include "AYFontRenderer.h"

int main()
{
	std::locale::global(std::locale("en_US.UTF-8"));
    //getchar();
	AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();



	return 0;
}