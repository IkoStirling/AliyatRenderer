#pragma once
#include "AYTexture.h"
#include "AYAnimatedSprite.h"
#include <unordered_map>

struct AnimeDataCreator
{
    std::string animeName;
    int beginIndex;
    int count;
};

class AYAnimationManager {
public:
    using AnimationCallback = std::function<void()>;
public:
    AYAnimationManager(AYRenderDevice* device);
    void shutdown();
    // 加载图集并注册动画
    std::shared_ptr<AYSpriteAtlas> loadAtlas(const std::string& atlasName, 
        const std::string& texturePath,
        const glm::vec2& spriteSize,
        const std::vector<std::pair<std::string, std::vector<AYAnimationFrame>>>& animations,
        const std::vector<bool>& loops = {});

    std::shared_ptr<AYSpriteAtlas> getAtlas(const std::string& atlasName);

    std::vector<AYAnimationFrame> makeFrames(
        int beginIndex,
        int frameCount,
        const glm::vec2& spriteSize,
        const glm::vec2& atlasSize,
        float duration = 0.1);

    std::vector<std::pair<std::string, std::vector<AYAnimationFrame>>> makeAnimationData(
        const glm::vec2& spriteSize,
        const glm::vec2& atlasSize,
        const std::vector<AnimeDataCreator>& indexs,
        float duration = 0.1
    );

    // 注册动画回调
    void registerAnimationEvent(const std::string& atlasName,
        const std::string& animationName,
        int frameIndex,
        AnimationCallback callback);

private:
    std::unordered_map<std::string, std::shared_ptr<AYSpriteAtlas>> _atlasMap;
    AYRenderDevice* _device;
};
