#pragma once
#include "BaseRendering/Camera/AY2DCamera.h"


void AY2DCamera::update(float delta_time)
{
    _lastTransform = _transform;

    float scaleWidth = _viewport.z / _zoom;  
    float scaleHeight = _viewport.w / _zoom; 

    // 1. 计算目标点在屏幕空间的位置
    glm::vec2 screenPos = glm::vec2(_targetPosition - _transform.position) + getScreenCenter();
    // 2. 计算死区边界（像素坐标）
    glm::vec2 deadzoneMin(
        scaleWidth * _deadzone.x,
        scaleHeight * _deadzone.z);
    glm::vec2 deadzoneMax(
        scaleWidth * _deadzone.y,
        scaleHeight * _deadzone.w);

    // 3. 计算移动偏移
    glm::vec2 moveOffset(0.0f);
    if (screenPos.x < deadzoneMin.x) moveOffset.x = screenPos.x - deadzoneMin.x;
    else if (screenPos.x > deadzoneMax.x) moveOffset.x = screenPos.x - deadzoneMax.x;

    if (screenPos.y < deadzoneMin.y) moveOffset.y = screenPos.y - deadzoneMin.y;
    else if (screenPos.y > deadzoneMax.y) moveOffset.y = screenPos.y - deadzoneMax.y;

    // 应用移动
    glm::vec2 newPos = glm::vec2(_transform.position) + moveOffset * delta_time * _moveSpeed;

    // 地图边界约束
    newPos.x = glm::clamp(newPos.x,
        _mapBounds.x + scaleWidth * _deadzone.x,
        _mapBounds.y - scaleWidth * (1 - _deadzone.y));

    newPos.y = glm::clamp(newPos.y,
        _mapBounds.z + scaleHeight * _deadzone.z,
        _mapBounds.w - scaleHeight * (1 - _deadzone.w));
    //std::cout << "screenPos: (" << screenPos.x << ", " << screenPos.y << ")\n";

    _transform.position = glm::vec3(newPos,1.f);

}

glm::mat4 AY2DCamera::getViewMatrix() const
{
    // 将屏幕中心定义为(0,0), 正常是在左上角, 不进行缩放
    return glm::translate(glm::mat4(1.0f),
        -glm::vec3(
            glm::vec2(_transform.position) + _additionalOffset,
            0.0f));
}

glm::mat4 AY2DCamera::getProjectionMatrix() const
{
    float zoomedWidth = _viewport.z / _zoom;  // 视口宽度（物理单位）
    float zoomedHeight = _viewport.w / _zoom; // 视口高度（物理单位）
    return glm::ortho(
        -zoomedWidth * 0.5f,  // left
        zoomedWidth * 0.5f,   // right
        -zoomedHeight * 0.5f,  // bottom 
        zoomedHeight * 0.5f,   // top
        _near,                
        _far                  
    );
}

void AY2DCamera::setViewBox(float near, float far)
{
    _near = near;
    _far = far;
}
