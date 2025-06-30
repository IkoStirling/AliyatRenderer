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

    void update(float delta_time)
    {

    }

    glm::mat4 getViewMatrix() const override 
    {
        return glm::translate(glm::mat4(1.0f),
            -glm::vec3(_currentPos, 0.0f));
    }

    glm::mat4 getProjectionMatrix() const override 
    {
        return glm::ortho(0.0f, _viewport.z, _viewport.w, 0.0f, -1.0f, 1.0f);
    }

    void setLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
    {
        glm::lookAt(eye, center, up);
    }

private:
    glm::vec2 _targetPos{ 0.0f };
    glm::vec2 _currentPos{ 0.0f };
    glm::vec4 _deadzone{ 0.3f, 0.7f, 0.3f, 0.7f }; // 屏幕比例死区(摄像机不会跟随的四边形区域）
};