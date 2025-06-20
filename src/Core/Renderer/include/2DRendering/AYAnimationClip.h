#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <memory>

struct AYAnimationFrame {
    glm::vec2 uvOffset;    // 纹理坐标偏移
    glm::vec2 uvSize;      // 纹理坐标尺寸
    float duration;        // 帧持续时间(秒)

    // 可添加事件触发点等扩展字段
};

/*
    动画切片，包含多帧，
*/
class AYAnimationClip {
public:
    AYAnimationClip(const std::string& name, bool isLoop = true);

    void addFrame(const AYAnimationFrame& frame);
    const AYAnimationFrame& getFrame(size_t index) const;
    size_t frameCount() const;

    std::string name;
    bool isLoop;

private:
    std::vector<AYAnimationFrame> _frames;
};