#pragma once
#include "IAYCamera.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"


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
    const glm::vec3& getFront() const;
    const glm::vec3& getUp() const;
    const glm::vec3& getRight() const;

private:
    void _updateCachedVectors();
    // 相机参数
    glm::vec3 _cachedFront;
    glm::vec3 _cachedRight;
    glm::vec3 _cachedUp;

    // 旋转角度
    float _yaw = -90.0f;   // 偏航角
    float _pitch = 0.0f;   // 俯仰角
    bool _flipY = true;

    // 投影参数
    bool _orthographic = false;
    float _fov = 60.0f;
    float _orthoSize = 5.0f;
    float _nearPlane = 0.1f;
    float _farPlane = 5000.f;

    // 跟随参数
    bool _smoothFollow = false;
    float _followSpeed = 5.0f;
};