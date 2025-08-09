#pragma once
#include "BaseRendering/Camera/AY3DCamera.h"

void AY3DCamera::update(float delta_time)
{
    _lastTransform = _transform;
    // 平滑跟随目标位置
    if (_smoothFollow)
    {
        _transform.position += (_targetPosition - _transform.position) * _followSpeed * delta_time;
    }
    else
    {
        _transform.position = _targetPosition;
    }
}

glm::mat4 AY3DCamera::getViewMatrix() const
{
    return glm::lookAt(
        _transform.position,              // 相机位置
        _transform.position + _cachedFront,     // 目标位置（位置+前向向量）
        _cachedUp // 固定世界空间的上向量
    );
    return glm::inverse(_transform.getTransformMatrix());
}

glm::mat4 AY3DCamera::getProjectionMatrix() const
{
    float aspect = _viewport.z / _viewport.w;
    return _orthographic
        ? glm::ortho(-_orthoSize * aspect, _orthoSize * aspect,
            -_orthoSize, _orthoSize,
            _nearPlane, _farPlane)
        : glm::perspective(glm::radians(_fov), aspect, _nearPlane, _farPlane);
}

void AY3DCamera::setLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
    _targetPosition = center;
    _cachedFront = glm::normalize(center - eye);
    _cachedUp = up;
}

void AY3DCamera::setPerspective(float fov, float nearPlane, float farPlane)
{
    _fov = fov;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _orthographic = false;
}

void AY3DCamera::setOrthographic(float size, float nearPlane, float farPlane)
{
    _orthoSize = size;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _orthographic = true;
}

void AY3DCamera::setPosition(const glm::vec3& position)
{
    _transform.position = position;
    _targetPosition = position;
}

void AY3DCamera::setTargetPosition(const glm::vec3& target)
{
    _targetPosition = target;
}

void AY3DCamera::rotate(float yaw, float pitch)
{
    // 更新相机的朝向
    _yaw += yaw;
    _pitch = glm::clamp(_pitch - pitch, -89.0f, 89.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    front.y = sin(glm::radians(_pitch));
    front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _cachedFront = glm::normalize(front);

    // 重新计算右向量和上向量
    _cachedRight = glm::normalize(glm::cross(_cachedFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    _cachedUp = glm::normalize(glm::cross(_cachedRight, _cachedFront));
}

void AY3DCamera::move(const glm::vec3& offset)
{
    _transform.position += offset;
}

const glm::vec3& AY3DCamera::getFront() const
{
    return _cachedFront;
}
const glm::vec3& AY3DCamera::getUp() const
{
    return _cachedUp;
}
const glm::vec3& AY3DCamera::getRight() const
{
    return _cachedRight;
}

void AY3DCamera::_updateCachedVectors()
{
    // 从 STTransform 的旋转计算 front
    _cachedFront.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _cachedFront.y = sin(glm::radians(_pitch));
    _cachedFront.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _cachedFront = glm::normalize(_cachedFront);

    // 计算 right 和 up
    _cachedRight = glm::normalize(glm::cross(_cachedFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    _cachedUp = glm::normalize(glm::cross(_cachedRight, _cachedFront));
}