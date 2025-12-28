#pragma once
#include "AYMathType.h"
namespace ayt::engine::render
{
    enum class LightType {
        Directional,
        Point,
        Spot
    };

    //struct STBaseLight
    //{
    //    math::Vector3 color = math::Vector3(1.0f);
    //    float intensity = 1.0f;
    //};

    struct DirectionalLight
    {
        math::Vector3 color = math::Vector3(1.0f);
        float intensity = 1.0f;
        math::Vector3 direction = math::Vector3(0.0f, -1.0f, 0.0f);
    };

    struct PointLight
    {
        math::Vector3 color = math::Vector3(1.0f);
        float intensity = 1.0f;
        math::Vector3 position = math::Vector3(0.0f);
        float radius = 10.0f;

        // 衰减参数
        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;
    };

    struct SpotLight //聚光灯
    {
        math::Vector3 color = math::Vector3(1.0f);
        float intensity = 1.0f;
        math::Vector3 position = math::Vector3(0.0f);
        math::Vector3 direction = math::Vector3(0.0f, 0.0f, -1.0f);
        float cutOff = glm::cos(glm::radians(12.5f));
        float outerCutOff = glm::cos(glm::radians(15.0f));

        // 衰减参数
        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;
    };
}