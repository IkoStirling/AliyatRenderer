#pragma once
#include "Core/ResourceManager/IAYResource.h"
#include "Core/ResourceManager/AYResourceRegistry.h"
#include "Core/MemoryPool/AYMemoryPool.h"
#include <iostream>


class AYTexture : public IAYResource
{
	SUPPORT_MEMORY_POOL(AYTexture)
	DECLARE_RESOURCE_CLASS(AYTexture, "textures")
public:
	virtual bool load(const std::string& filepath)override
	{
		IAYResource::load(filepath);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Texture load" << std::endl;
		return true;
	}
	virtual bool unload()override
	{
		std::cout << "Texture unload" << std::endl;
		return true;
	}
	virtual bool reload(const std::string& filepath)override
	{
		std::cout << "Texture reload" << std::endl;
		return true;
	}
	virtual size_t sizeInBytes()override
	{
		return 0;
	}

private:

};

REGISTER_RESOURCE_CLASS(AYTexture)