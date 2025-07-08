#pragma once
#include "IAYCamera.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "STTransform.h"


class AY3DCamera : public IAYCamera
{
public:
    IAYCamera::Type getType() const override
    {
        return IAYCamera::Type::PERSPECTIVE_3D;
    }

    void update(float delta_time);

    glm::mat4 getViewMatrix() const override;

    glm::mat4 getProjectionMatrix() const override;

    void setLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

    void setPerspective(float fov, float nearPlane, float farPlane);

    void setOrthographic(float size, float nearPlane, float farPlane);

    void setPosition(const glm::vec3& position);

    void setTargetPosition(const glm::vec3& target);

    void rotate(float yaw, float pitch);

    void move(const glm::vec3& offset);

    // 获取相机属性
    glm::vec3 getPosition() const { return _position; }
    glm::vec3 getFront() const { return _front; }
    glm::vec3 getUp() const { return _up; }
    glm::vec3 getRight() const { return _right; }

private:
    // 相机参数
    glm::mat4 _viewMatrix;
    glm::vec3 _position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 _targetPosition{ 0.0f, 0.0f, 0.0f };
    glm::vec3 _front{ 0.0f, 0.0f, -1.0f };
    glm::vec3 _up{ 0.0f, 1.0f, 0.0f };
    glm::vec3 _right{ 1.0f, 0.0f, 0.0f };

    // 旋转角度
    float _yaw = -90.0f;   // 偏航角
    float _pitch = 0.0f;   // 俯仰角

    // 投影参数
    bool _orthographic = false;
    float _fov = 80.0f;
    float _orthoSize = 5.0f;
    float _nearPlane = 0.1f;
    float _farPlane = 5000.0f;

    // 跟随参数
    bool _smoothFollow = false;
    float _followSpeed = 5.0f;
};