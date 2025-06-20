#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <memory>

struct AYAnimationFrame {
    glm::vec2 uvOffset;    // ��������ƫ��
    glm::vec2 uvSize;      // ��������ߴ�
    float duration;        // ֡����ʱ��(��)

    // ������¼����������չ�ֶ�
};

/*
    ������Ƭ��������֡��
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