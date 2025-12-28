#pragma once
#include "AYTextureManager.h"
#include "AYRenderDevice.h"
namespace ayt::engine::render
{
	using namespace ::ayt::engine::resource;
	TextureManager::TextureManager(RenderDevice* device) : _device(device) {}

	void TextureManager::shutdown()
	{

	}

	uint32_t TextureManager::getTexture(const std::string& path)
	{
		auto it = _textures.find(path);
		if (it != _textures.end())
			return it->second;
		auto texture = ResourceManager::getInstance().load<AYTexture>(path);
		if (!texture)
			throw std::runtime_error("[TextureManager] texture load failed");
		auto id = _device->createTexture2D(texture->getPixelData(), texture->getWidth(), texture->getHeight(), texture->getChannels());
		_textures.emplace(path, id);
	}
}