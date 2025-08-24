#pragma once
#include "AYTextureManager.h"
#include "AYRenderDevice.h"

AYTextureManager::AYTextureManager(AYRenderDevice* device) : _device(device) {}

void AYTextureManager::shutdown()
{

}

uint32_t AYTextureManager::getTexture(const std::string& path)
{
	auto it = _textures.find(path);
	if (it != _textures.end())
		return it->second;
	auto texture = AYResourceManager::getInstance().load<AYTexture>(path);
	if (!texture)
		throw std::runtime_error("[AYTextureManager] texture load failed");
	auto id = _device->createTexture2D(texture->getPixelData(), texture->getWidth(), texture->getHeight(), texture->getChannels());
	_textures.emplace(path, id);
}