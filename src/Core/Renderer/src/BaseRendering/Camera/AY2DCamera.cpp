#pragma once
#include "BaseRendering/Camera/AY2DCamera.h"

void AY2DCamera::update(float delta_time)
{
    // 1. ����Ŀ�������Ļ�ռ��λ��
    glm::vec2 screenPos = glm::vec2(_targetPosition - _transform.position) + getScreenCenter();
    // 2. ���������߽磨�������꣩
    glm::vec2 deadzoneMin(_viewport.z * _deadzone.x, _viewport.w * _deadzone.z);
    glm::vec2 deadzoneMax(_viewport.z * _deadzone.y, _viewport.w * _deadzone.w);

    // 3. �����ƶ�ƫ��
    glm::vec2 moveOffset(0.0f);
    if (screenPos.x < deadzoneMin.x) moveOffset.x = screenPos.x - deadzoneMin.x;
    else if (screenPos.x > deadzoneMax.x) moveOffset.x = screenPos.x - deadzoneMax.x;

    if (screenPos.y < deadzoneMin.y) moveOffset.y = screenPos.y - deadzoneMin.y;
    else if (screenPos.y > deadzoneMax.y) moveOffset.y = screenPos.y - deadzoneMax.y;

    // Ӧ���ƶ�
    glm::vec2 newPos = glm::vec2(_transform.position) + moveOffset * delta_time * _moveSpeed;

    // ��ͼ�߽�Լ��
    newPos.x = glm::clamp(newPos.x,
        _mapBounds.x + _viewport.z * _deadzone.x,
        _mapBounds.y - _viewport.z * (1 - _deadzone.y));

    newPos.y = glm::clamp(newPos.y,
        _mapBounds.z + _viewport.w * _deadzone.z,
        _mapBounds.w - _viewport.w * (1 - _deadzone.w));
    //std::cout << "screenPos: (" << screenPos.x << ", " << screenPos.y << ")\n";

    _transform.position = glm::vec3(newPos,1.f);
}

glm::mat4 AY2DCamera::getViewMatrix() const
{
    // ����Ļ���Ķ���Ϊ(0,0), �����������Ͻ�
    return glm::translate(glm::mat4(1.0f),
        -glm::vec3(
            glm::vec2(_transform.position) + _additionalOffset,
            0.0f));
}

glm::mat4 AY2DCamera::getProjectionMatrix() const
{
    return glm::ortho(
        -_viewport.z * 0.5f,  // left
        _viewport.z * 0.5f,   // right
        -_viewport.w * 0.5f,  // bottom 
        _viewport.w * 0.5f,   // top
        _near,                
        _far                  
    );
}

void AY2DCamera::setViewBox(float near, float far)
{
    _near = near;
    _far = far;
}
