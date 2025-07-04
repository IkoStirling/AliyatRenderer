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

    void update(float delta_time)
    {
        // ƽ������Ŀ��λ��
        if (_smoothFollow)
        {
            _position += (_targetPosition - _position) * _followSpeed * delta_time;
        }
        else
        {
            _position = _targetPosition;
        }
    }

    glm::mat4 getViewMatrix() const override 
    {
        return glm::lookAt(
            _position,              // ���λ��
            _position + _front,     // Ŀ��λ�ã�λ��+ǰ��������
            _up // �̶�����ռ��������
        );
    }

    glm::mat4 getProjectionMatrix() const override 
    {
        float aspect = _viewport.z / _viewport.w;
        return _orthographic
            ? glm::ortho(-_orthoSize * aspect, _orthoSize * aspect,
                -_orthoSize, _orthoSize,
                _nearPlane, _farPlane)
            : glm::perspective(glm::radians(_fov), aspect, _nearPlane, _farPlane);
    }

    void setLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
    {
        _position = eye;
        _targetPosition = center;
        _front = glm::normalize(center - eye);
        _up = up;
        _viewMatrix = glm::lookAt(eye, center, up);
    }

    void setPerspective(float fov, float nearPlane, float farPlane)
    {
        _fov = fov;
        _nearPlane = nearPlane;
        _farPlane = farPlane;
        _orthographic = false;
    }

    void setOrthographic(float size, float nearPlane, float farPlane)
    {
        _orthoSize = size;
        _nearPlane = nearPlane;
        _farPlane = farPlane;
        _orthographic = true;
    }

    void setPosition(const glm::vec3& position)
    {
        _position = position;
        _targetPosition = position;
    }

    void setTargetPosition(const glm::vec3& target)
    {
        _targetPosition = target;
        _front = glm::normalize(target - _position);
    }

    void rotate(float yaw, float pitch)
    {
        // ��������ĳ���
        _yaw += yaw;
        _pitch = glm::clamp(_pitch + pitch, -89.0f, 89.0f);

        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front = glm::normalize(front);

        // ���¼�����������������
        _right = glm::normalize(glm::cross(_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        _up = glm::normalize(glm::cross(_right, _front));
    }

    void move(const glm::vec3& offset)
    {
        _position += offset;
        _targetPosition += offset;
    }

    // ��ȡ�������
    glm::vec3 getPosition() const { return _position; }
    glm::vec3 getFront() const { return _front; }
    glm::vec3 getUp() const { return _up; }
    glm::vec3 getRight() const { return _right; }

private:
    // �������
    glm::mat4 _viewMatrix;
    glm::vec3 _position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 _targetPosition{ 0.0f, 0.0f, 0.0f };
    glm::vec3 _front{ 0.0f, 0.0f, -1.0f };
    glm::vec3 _up{ 0.0f, 1.0f, 0.0f };
    glm::vec3 _right{ 1.0f, 0.0f, 0.0f };

    // ��ת�Ƕ�
    float _yaw = -90.0f;   // ƫ����
    float _pitch = 0.0f;   // ������

    // ͶӰ����
    bool _orthographic = false;
    float _fov = 80.0f;
    float _orthoSize = 5.0f;
    float _nearPlane = 0.1f;
    float _farPlane = 5000.0f;

    // �������
    bool _smoothFollow = false;
    float _followSpeed = 5.0f;
};