#pragma once
#include <glm/glm.hpp>

enum class LightType {
    Directional,
    Point,
    Spot
};

//struct STBaseLight
//{
//    glm::vec3 color = glm::vec3(1.0f);
//    float intensity = 1.0f;
//};

struct STDirectionalLight
{
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
};

struct STPointLight
{ 
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    glm::vec3 position = glm::vec3(0.0f);
    float radius = 10.0f;

    // 衰减参数
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

struct STSpotLight //聚光灯
{
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(15.0f));

    // 衰减参数
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};