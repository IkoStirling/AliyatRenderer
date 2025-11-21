#pragma once
#include "AYMathType.h"

enum class LightType {
    Directional,
    Point,
    Spot
};

//struct STBaseLight
//{
//    AYMath::Vector3 color = AYMath::Vector3(1.0f);
//    float intensity = 1.0f;
//};

struct STDirectionalLight
{
    AYMath::Vector3 color = AYMath::Vector3(1.0f);
    float intensity = 1.0f;
    AYMath::Vector3 direction = AYMath::Vector3(0.0f, -1.0f, 0.0f);
};

struct STPointLight
{ 
    AYMath::Vector3 color = AYMath::Vector3(1.0f);
    float intensity = 1.0f;
    AYMath::Vector3 position = AYMath::Vector3(0.0f);
    float radius = 10.0f;

    // 衰减参数
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

struct STSpotLight //聚光灯
{
    AYMath::Vector3 color = AYMath::Vector3(1.0f);
    float intensity = 1.0f;
    AYMath::Vector3 position = AYMath::Vector3(0.0f);
    AYMath::Vector3 direction = AYMath::Vector3(0.0f, 0.0f, -1.0f);
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(15.0f));

    // 衰减参数
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};