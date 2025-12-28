#pragma once
#include "AYResourceManager.h"
#include <unordered_map>
namespace ayt::engine::render
{
	class RenderDevice;

	class TextureManager
	{
	public:
		TextureManager(RenderDevice* device);

		void shutdown();

		uint32_t getTexture(const std::string& path);

		void outputAllTexture()
		{
			for (auto [path, id] : _textures)
			{
				static int x = 0;
				auto texture = resource::ResourceManager::getInstance().load<resource::AYTexture>(path);
				cv::imwrite(std::string("tex/test_" + x++) + ".png", cv::Mat(texture->getImageData()));

			}
		}
	private:
		RenderDevice* _device = nullptr;
		std::unordered_map<std::string, uint32_t> _textures;
	};
}