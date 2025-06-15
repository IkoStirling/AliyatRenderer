#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class AYSpriteAnimator {
public:
    struct AnimationFrame {
        glm::vec2 uvOffset;    // ��������ƫ��
        glm::vec2 uvSize;      // ��������ߴ�
        float duration;        // ֡����ʱ��(��)
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
    glm::vec2 _spriteSize;     // ������������سߴ�
    glm::vec2 _atlasSize;      // ����ͼ�������سߴ�
    std::unordered_map<std::string, Animation> _animations;

    Animation* _currentAnimation = nullptr;
    size_t _currentFrameIndex = 0;
    float _frameTimer = 0.0f;
};