#pragma once
#include "IAYCamera.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"


class AY2DCamera : public IAYCamera
{
public:
    IAYCamera::Type getType() const override
    {
        return IAYCamera::Type::ORTHOGRAPHIC_2D;
    }

    void update(float delta_time) 
    {
        // 1. 计算目标点在屏幕空间的位置
        glm::vec2 screenPos = _targetPos - _currentPos + getScreenCenter();
        // 2. 计算死区边界（像素坐标）
        glm::vec2 deadzoneMin(_viewport.z * _deadzone.x, _viewport.w * _deadzone.z);
        glm::vec2 deadzoneMax(_viewport.z * _deadzone.y, _viewport.w * _deadzone.w);

        // 3. 计算移动偏移
        glm::vec2 moveOffset(0.0f);
        if (screenPos.x < deadzoneMin.x) moveOffset.x = screenPos.x - deadzoneMin.x;
        else if (screenPos.x > deadzoneMax.x) moveOffset.x = screenPos.x - deadzoneMax.x;

        if (screenPos.y < deadzoneMin.y) moveOffset.y = screenPos.y - deadzoneMin.y;
        else if (screenPos.y > deadzoneMax.y) moveOffset.y = screenPos.y - deadzoneMax.y;

        // 应用移动
        glm::vec2 newPos = _currentPos + moveOffset * delta_time * _moveSpeed;

        // 地图边界约束
        newPos.x = glm::clamp(newPos.x,
            _mapBounds.x + _viewport.z * _deadzone.x,
            _mapBounds.y - _viewport.z * (1 - _deadzone.y));

        newPos.y = glm::clamp(newPos.y,
            _mapBounds.z + _viewport.w * _deadzone.z,
            _mapBounds.w - _viewport.w * (1 - _deadzone.w));
        //std::cout << "screenPos: (" << screenPos.x << ", " << screenPos.y << ")\n";

        _currentPos = newPos;
    }

    glm::mat4 getViewMatrix() const override 
    {
        // 将屏幕中心定义为(0,0), 正常是在左上角
        return glm::translate(glm::mat4(1.0f),
            -glm::vec3(
                _currentPos - getScreenCenter() + _additionalOffset,
                0.0f));
    }

    glm::mat4 getProjectionMatrix() const override 
    {
        return glm::ortho(
            0.f,            //left
            _viewport.z,    //right
            0.f,            //bottom
            _viewport.w,    //top
            -1.f,           //near
            1.f             //far
        );           
    }

    void setDeadzone(const glm::vec4& zone) { _deadzone = zone; } // left, right, bottom, top
    void setTargetPosition(const glm::vec2& targetPos) { _targetPos = targetPos; }
    void setCurrentPosition(const glm::vec2& currentPos) { _currentPos = currentPos; }
    glm::vec2 getScreenCenter() const
    {
        return  glm::vec2(_viewport.z, _viewport.w) * 0.5f;
    }
private:
    glm::vec2 _targetPos{ 0.0f };
    glm::vec2 _currentPos{ 0.0f };
    glm::vec4 _deadzone{ 0.3f, 0.7f, 0.3f, 0.7f }; // 屏幕比例死区(摄像机不会跟随的四边形区域）
    glm::vec4 _mapBounds{ -5000.f, 5000.f, -5000.f, 5000.f };

    float _moveSpeed = 5.f;
};