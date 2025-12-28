#pragma once
#include "BaseRendering/Camera/AYCamera3D.h"
namespace ayt::engine::render
{

    void Camera3D::update(float delta_time)
    {
        if (math::Transform::approximatelyEqual(_targetPosition, _transform.position))
            return;
        else
            _dirtyView = true;
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

    math::Matrix4 Camera3D::getViewMatrix() const
    {
        if (_dirtyView)
        {
            _cachedView = glm::lookAt(
                _transform.position,              // 相机位置
                _transform.position + _cachedFront,     // 目标位置（位置+前向向量）
                _cachedUp // 固定世界空间的上向量
            );
            _dirtyView = false;
            //return glm::inverse(_transform.getTransformMatrix());
        }
        return _cachedView;
    }

    math::Matrix4 Camera3D::getProjectionMatrix() const
    {
        if (_dirtyProjection)
        {
            float aspect = _viewport.z / _viewport.w;
            if (_orthographic) {
                float zoomedSize = _orthoSize / _zoom;
                _cachedProjection = glm::ortho(-zoomedSize * aspect, zoomedSize * aspect,
                    -zoomedSize, zoomedSize,
                    _nearPlane, _farPlane);
            }
            else {
                // 透视投影 - 通过调整FOV实现缩放
                float zoomedFov = glm::radians(_fov) / _zoom;
                _cachedProjection = glm::perspective(glm::clamp(zoomedFov, glm::radians(1.0f), glm::radians(179.0f)),
                    aspect, _nearPlane, _farPlane);
            }
            _dirtyProjection = false;
        }
        return _cachedProjection;
    }

    void Camera3D::setLookAt(const math::Vector3& eye, const math::Vector3& center, const math::Vector3& up)
    {
        _targetPosition = center;
        _cachedFront = glm::normalize(center - eye);
        _cachedUp = up;
    }

    void Camera3D::setPerspective(float fov, float nearPlane, float farPlane)
    {
        _dirtyProjection = true;
        _fov = fov;
        _nearPlane = nearPlane;
        _farPlane = farPlane;
        _orthographic = false;
    }

    void Camera3D::setOrthographic(float size, float nearPlane, float farPlane)
    {
        _dirtyProjection = true;
        _orthoSize = size;
        _nearPlane = nearPlane;
        _farPlane = farPlane;
        _orthographic = true;
    }

    void Camera3D::setPosition(const math::Vector3& position)
    {
        _transform.position = position;
        _targetPosition = position;
    }

    void Camera3D::setTargetPosition(const math::Vector3& target)
    {
        _targetPosition = target;
    }

    void Camera3D::rotate(float yaw, float pitch)
    {
        _dirtyView = true;
        // 更新相机的朝向
        _yaw += yaw;
        _pitch = glm::clamp(_pitch - pitch, -89.0f, 89.0f);

        math::Vector3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _cachedFront = glm::normalize(front);

        // 重新计算右向量和上向量
        _cachedRight = glm::normalize(glm::cross(_cachedFront, math::Vector3(0.0f, 1.0f, 0.0f)));
        _cachedUp = glm::normalize(glm::cross(_cachedRight, _cachedFront));
    }

    void Camera3D::move(const math::Vector3& offset)
    {
        _transform.position += offset;
    }

    const math::Vector3& Camera3D::getFront() const
    {
        return _cachedFront;
    }
    const math::Vector3& Camera3D::getUp() const
    {
        return _cachedUp;
    }
    const math::Vector3& Camera3D::getRight() const
    {
        return _cachedRight;
    }

    void Camera3D::_updateCachedVectors()
    {
        // 从 math::Transform 的旋转计算 front
        _cachedFront.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _cachedFront.y = sin(glm::radians(_pitch));
        _cachedFront.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _cachedFront = glm::normalize(_cachedFront);

        // 计算 right 和 up
        _cachedRight = glm::normalize(glm::cross(_cachedFront, math::Vector3(0.0f, 1.0f, 0.0f)));
        _cachedUp = glm::normalize(glm::cross(_cachedRight, _cachedFront));
    }
}