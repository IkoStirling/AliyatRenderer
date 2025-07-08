#pragma once
#include <glm/glm.hpp>


class IAYCamera {
public:
    struct OrthoInfo
    {
        float left = 0.f;
        float right = 1920.f;
        float bottom = 0.f;
        float top = 1080.f;
        float near = -100.f;
        float far = 100.f;
    };

    virtual ~IAYCamera() = default;

    // ��ȡ��ͼ����View Matrix��
    virtual glm::mat4 getViewMatrix() const = 0;

    // ��ȡͶӰ����Projection Matrix��
    virtual glm::mat4 getProjectionMatrix() const = 0;

    // ÿ֡���£���������չ��
    virtual void update(float delta_time) = 0;

    // ������ͱ�ʶ
    enum class Type { PERSPECTIVE_3D, ORTHOGRAPHIC_2D, CUSTOM };
    virtual Type getType() const = 0;

    // �ӿڿ���
    virtual void setViewport(const glm::vec4& viewport) { _viewport = viewport; }   //�������������Ⱦ�ӿ�
    glm::vec4 getViewport() const { return _viewport; }

    virtual void setAdditionalOffset(const glm::vec2& offset) { _additionalOffset = offset; }

protected:
    glm::vec4 _viewport{ 0, 0, 1920, 1080 }; // x,y,width,height
    glm::vec2 _additionalOffset{ 0.0f };
};