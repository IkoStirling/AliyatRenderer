#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class AYSpriteAnimator {
public:
    struct AnimationFrame {
        glm::vec2 uvOffset;    // 纹理坐标偏移
        glm::vec2 uvSize;      // 纹理坐标尺寸
        float duration;        // 帧持续时间(秒)
    };

    struct Animation {
        std::string name;
        std::vector<AnimationFrame> frames;
        bool loop = true;
    };

    AYSpriteAnimator(const glm::vec2& spriteSize, const glm::vec2& atlasSize);

    void addAnimation(const std::string& name,
        const std::vector<AnimationFrame>& frames,
        bool loop = true);

    void play(const std::string& name, bool forceRestart = false);
    void update(float deltaTime);
    void stop();

    const AnimationFrame& getCurrentFrame() const;
    bool isPlaying() const { return _currentAnimation != nullptr; }

private:
    glm::vec2 _spriteSize;     // 单个精灵的像素尺寸
    glm::vec2 _atlasSize;      // 整个图集的像素尺寸
    std::unordered_map<std::string, Animation> _animations;

    Animation* _currentAnimation = nullptr;
    size_t _currentFrameIndex = 0;
    float _frameTimer = 0.0f;
};