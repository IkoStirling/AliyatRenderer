#pragma once
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"

struct STMaterial {
    enum class Type {
        Opaque,
        Transparent,
        Wireframe,
        None
    };

    glm::vec4 baseColor = glm::vec4(1.0f);    // 基础颜色 (RGBA)
    float metallic = 0.0f;                    // 金属度 (0-1)
    float roughness = 0.5f;                   // 粗糙度 (0-1)
    float ao = 1.0f;                          // 环境光遮蔽 (0-1)
    float reflectance = 0.5f;                 // 反射率 (0-1)
    float emissiveIntensity = 0.0f;           // 自发光强度
    glm::vec3 emissiveColor = glm::vec3(0.0f); // 自发光颜色

    // 纹理
    GLuint albedoTexture = 0;                 // 反照率贴图 (替代baseColor)
    GLuint normalTexture = 0;                 // 法线贴图
    GLuint metallicTexture = 0;               // 金属度贴图
    GLuint roughnessTexture = 0;              // 粗糙度贴图
    GLuint aoTexture = 0;                     // AO贴图
    GLuint emissiveTexture = 0;               // 自发光贴图

    Type type = Type::Opaque;

    // 纹理使用标志
    bool useAlbedoTexture = false;
    bool useNormalTexture = false;
    bool useMetallicTexture = false;
    bool useRoughnessTexture = false;
    bool useAOTexture = false;
    bool useEmissiveTexture = false;
};