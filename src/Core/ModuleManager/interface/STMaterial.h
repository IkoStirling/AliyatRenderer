#pragma once
#include "glm/glm.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <string>

struct STMaterial {
    enum class Type {
        Opaque,
        Transparent,
        Wireframe,
        None
    };

    uint32_t id = 0;
    std::string name = "";

    glm::vec4 baseColor = glm::vec4(1.0f);    // 基础颜色 (RGBA)
    float metallic = 0.0f;                    // 金属度 (0-1)
    float roughness = 0.5f;                   // 粗糙度 (0-1)
    float ao = 1.0f;                          // 环境光遮蔽 (0-1)
    float reflectance = 0.5f;                 // 反射率 (0-1)
    float emissiveIntensity = 0.0f;           // 自发光强度
    glm::vec3 emissiveColor = glm::vec3(0.0f); // 自发光颜色
    bool isTwoSided = false;                     // 双面渲染，默认背面剔除

    // 纹理
    std::string albedoTexture = "";             // 反照率贴图 (替代baseColor)
    std::string normalTexture = "";             // 法线贴图
    std::string metallicTexture = "";           // 金属度贴图
    std::string roughnessTexture = "";          // 粗糙度贴图
    std::string aoTexture = "";                 // AO贴图
    std::string emissiveTexture = "";           // 自发光贴图
    std::string opacityTexture = "";           // 透明度贴图

    Type type = Type::Opaque;
};