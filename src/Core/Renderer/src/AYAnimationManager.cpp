#include "AYAnimationManager.h"
#include "AYResourceManager.h"

AYAnimationManager::AYAnimationManager(AYRenderDevice* device) :
	_device(device)
{
}

std::shared_ptr<AYSpriteAtlas> AYAnimationManager::loadAtlas(const std::string& atlasName,
	const std::string& texturePath,
	const glm::vec2& spriteSize,
	const std::vector<std::pair<std::string, std::vector<AYAnimationFrame>>>& animations,
	const std::vector<bool>& loops)
{
	// 检查是否已加载
	if (auto it = _atlasMap.find(atlasName); it != _atlasMap.end()) {
		return it->second;
	}

	// 加载纹理
	auto tex = AYResourceManager::getInstance().load<AYTexture>(texturePath);
	if (!tex) return nullptr;

	GLuint textureId = _device->createTexture2D(tex->getPixelData(),
		tex->getWidth(),
		tex->getHeight(),
		tex->getChannels());

	// 存储图集数据
	auto atlas = std::make_shared<AYSpriteAtlas>(
		textureId,
		spriteSize,
		glm::vec2(tex->getWidth(), tex->getHeight())
	);

	// 添加动画切片
	size_t i = 0;
	for (const auto& [name, frames] : animations) {
		auto loop = i < loops.size() ? loops[i++] : false;
		auto clip = std::make_shared<AYAnimationClip>(name,loop);
		for (const auto& frame : frames) {
			clip->addFrame(frame);
		}
		atlas->addAnimation(name, clip);
	}

	_atlasMap[atlasName] = atlas;
	return atlas;
}

std::shared_ptr<AYSpriteAtlas> AYAnimationManager::getAtlas(const std::string& atlasName)
{
	auto it = _atlasMap.find(atlasName);
	return it != _atlasMap.end() ? it->second : nullptr;
}

std::vector<AYAnimationFrame> AYAnimationManager::makeFrames(
	int beginIndex,
	int frameCount,
	const glm::vec2& spriteSize,
	const glm::vec2& atlasSize,
	float duration)
{
	std::vector<AYAnimationFrame> outFrames;
	if (beginIndex < 0 || frameCount <= 0) return outFrames;

	// 计算图集的行列数
	const int cols = static_cast<int>(atlasSize.x / spriteSize.x);
	const int rows = static_cast<int>(atlasSize.y / spriteSize.y);
	const int totalFrames = cols * rows;

	// 验证参数有效性
	if (cols == 0 || rows == 0 || beginIndex >= totalFrames) {
		return outFrames;
	}

	// 计算结束索引（确保不越界）
	const int endIndex = std::min(beginIndex + frameCount, totalFrames);
	frameCount = endIndex - beginIndex; // 修正实际帧数

	// 预分配内存
	outFrames.reserve(frameCount);

	// 计算每个帧的UV坐标
	for (int i = 0; i < frameCount; ++i) {
		const int index = beginIndex + i;
		const int row = index / cols;  // 计算行号
		const int col = index % cols;  // 计算列号

		outFrames.push_back({
			glm::vec2(col * spriteSize.x / atlasSize.x,  // u offset
					 row * spriteSize.y / atlasSize.y),  // v offset
			glm::vec2(spriteSize.x / atlasSize.x,        // u size
					 spriteSize.y / atlasSize.y),        // v size
			duration                                    // frame duration
			});
	}

	return outFrames;
}

std::vector<std::pair<std::string, std::vector<AYAnimationFrame>>> 
AYAnimationManager::makeAnimationData(
	const glm::vec2& spriteSize,
	const glm::vec2& atlasSize,
	std::vector<AnimeDataCreator> indexs,
	float duration)
{
	std::vector<std::pair<std::string, std::vector<AYAnimationFrame>>> data;
	for (auto index : indexs)
	{
		data.push_back({
			index.animeName,
			makeFrames(index.beginIndex,index.count,spriteSize,atlasSize,duration)
			});
	}
	return data;
}



void AYAnimationManager::registerAnimationEvent(const std::string& atlasName, const std::string& animationName, int frameIndex, AnimationCallback callback)
{
}
