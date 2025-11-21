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

    AYMath::Matrix4 getViewMatrix() const override;

    AYMath::Matrix4 getProjectionMatrix() const override;

    void setLookAt(const AYMath::Vector3& eye, const AYMath::Vector3& center, const AYMath::Vector3& up);

    void setPerspective(float fov, float nearPlane, float farPlane);

    void setOrthographic(float size, float nearPlane, float farPlane);

    void setPosition(const AYMath::Vector3& position);

    void setTargetPosition(const AYMath::Vector3& target);

    void rotate(float yaw, float pitch);

    void move(const AYMath::Vector3& offset);

    // 获取相机属性
    const AYMath::Vector3& getFront() const;
    const AYMath::Vector3& getUp() const;
    const AYMath::Vector3& getRight() const;

private:
    void _updateCachedVectors();
    // 相机参数
    AYMath::Vector3 _cachedFront;
    AYMath::Vector3 _cachedRight;
    AYMath::Vector3 _cachedUp;

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