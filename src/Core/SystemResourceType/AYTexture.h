#pragma once
#include "Core/ResourceManager/IAYResource.h"
#include <iostream>
class AYTexture : public IAYResource
{
public:
	virtual bool load(const std::string& filepath)override
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));
		std::cout << "Texture load" << std::endl;
		return true;
	}
	virtual bool unload()override
	{
		std::cout << "Texture unload" << std::endl;
		return true;
	}

	bool test{ false };
private:

};