#pragma once
#include "AYResourceManager.h"
#include <unordered_map>

class AYRenderDevice;

class AYTextureManager
{
public:
	AYTextureManager(AYRenderDevice* device);

	void shutdown();

	uint32_t getTexture(const std::string& path);

	void outputAllTexture()
	{
		for (auto [path, id] : _textures)
		{
			static int x = 0;
			auto texture = AYResourceManager::getInstance().load<AYTexture>(path);
			cv::imwrite(std::string("tex/test_" + x++)  + ".png", cv::Mat(texture->getImageData()));

		}
	}
private:
	AYRenderDevice* _device = nullptr;
	std::unordered_map<std::string, uint32_t> _textures;
};