#pragma once
#include "AYAnimationClip.h"
#include <unordered_map>
#include <memory>
#include "GLFW/glfw3.h"

/*
    ����ͼ����һ������ͼ�����԰������������Ƭ������ͼ����Դ��������
*/
class AYSpriteAtlas {
public:
    AYSpriteAtlas(GLuint textureId,
        const glm::vec2& spriteSize,
        const glm::vec2& atlasSize);

    bool addAnimation(const std::string& name,
        std::shared_ptr<AYAnimationClip> clip);

    std::shared_ptr<AYAnimationClip> getAnimation(const std::string& name) const;
    GLuint getTexture() const { return _texture; }
    bool hasAnimation(const std::string& name) const { return _animations.find(name) != _animations.end(); }

private:
    GLuint _texture;
    glm::vec2 _spriteSize;
    glm::vec2 _atlasSize;
    std::unordered_map<std::string, std::shared_ptr<AYAnimationClip>> _animations;
};