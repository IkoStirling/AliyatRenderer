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

    glm::vec4 baseColor = glm::vec4(1.0f);    // ������ɫ (RGBA)
    float metallic = 0.0f;                    // ������ (0-1)
    float roughness = 0.5f;                   // �ֲڶ� (0-1)
    float ao = 1.0f;                          // �������ڱ� (0-1)
    float reflectance = 0.5f;                 // ������ (0-1)
    float emissiveIntensity = 0.0f;           // �Է���ǿ��
    glm::vec3 emissiveColor = glm::vec3(0.0f); // �Է�����ɫ

    // ����
    GLuint albedoTexture = 0;                 // ��������ͼ (���baseColor)
    GLuint normalTexture = 0;                 // ������ͼ
    GLuint metallicTexture = 0;               // ��������ͼ
    GLuint roughnessTexture = 0;              // �ֲڶ���ͼ
    GLuint aoTexture = 0;                     // AO��ͼ
    GLuint emissiveTexture = 0;               // �Է�����ͼ

    Type type = Type::Opaque;

    // ����ʹ�ñ�־
    bool useAlbedoTexture = false;
    bool useNormalTexture = false;
    bool useMetallicTexture = false;
    bool useRoughnessTexture = false;
    bool useAOTexture = false;
    bool useEmissiveTexture = false;
};