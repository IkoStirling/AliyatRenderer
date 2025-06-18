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
	// ����Ƿ��Ѽ���
	if (auto it = _atlasMap.find(atlasName); it != _atlasMap.end()) {
		return it->second;
	}

	// ��������
	auto tex = AYResourceManager::getInstance().load<AYTexture>(texturePath);
	if (!tex) return nullptr;

	GLuint textureId = _device->createTexture2D(tex->getPixelData(),
		tex->getWidth(),
		tex->getHeight(),
		tex->getChannels());

	// �洢ͼ������
	auto atlas = std::make_shared<AYSpriteAtlas>(
		textureId,
		spriteSize,
		glm::vec2(tex->getWidth(), tex->getHeight())
	);

	// ��Ӷ�����Ƭ
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

	// ����ͼ����������
	const int cols = static_cast<int>(atlasSize.x / spriteSize.x);
	const int rows = static_cast<int>(atlasSize.y / spriteSize.y);
	const int totalFrames = cols * rows;

	// ��֤������Ч��
	if (cols == 0 || rows == 0 || beginIndex >= totalFrames) {
		return outFrames;
	}

	// �������������ȷ����Խ�磩
	const int endIndex = std::min(beginIndex + frameCount, totalFrames);
	frameCount = endIndex - beginIndex; // ����ʵ��֡��

	// Ԥ�����ڴ�
	outFrames.reserve(frameCount);

	// ����ÿ��֡��UV����
	for (int i = 0; i < frameCount; ++i) {
		const int index = beginIndex + i;
		const int row = index / cols;  // �����к�
		const int col = index % cols;  // �����к�

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
