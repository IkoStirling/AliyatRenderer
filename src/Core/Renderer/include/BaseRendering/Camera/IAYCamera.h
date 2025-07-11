#pragma once
#include <glm/glm.hpp>
#include "STTransform.h"

class IAYCamera {
public:
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

    virtual const glm::vec3 getPosition() const { return _transform.position; }
protected:
    STTransform _transform;
    glm::vec3 _targetPosition = glm::vec3(0.f);
    glm::vec4 _viewport{ 0, 0, 1920, 1080 }; // x,y,width,height
    glm::vec2 _additionalOffset{ 0.0f };
};