#pragma once
#include "glm/glm.hpp"
#include <vector>
#include <string>

struct STMesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec4> colors;
    std::vector<glm::ivec4> boneIDs;      // 每个顶点关联的骨骼ID（最多4个）
    std::vector<glm::vec4> boneWeights;
    std::vector<glm::mat4> boneOffsetMatrices;

    glm::vec3 boundingBoxMin;
    glm::vec3 boundingBoxMax;
    glm::vec3 center;
    float radius;
    int lodLevel;
    bool castShadow;

    std::vector<uint32_t> indices;
    std::string materialName;

    void calculateBoundingBox() {
        boundingBoxMin = glm::vec3(FLT_MAX);
        boundingBoxMax = glm::vec3(-FLT_MAX);
        for (const auto& vertex : vertices) {
            boundingBoxMin = glm::min(boundingBoxMin, vertex);
            boundingBoxMax = glm::max(boundingBoxMax, vertex);
        }
        center = (boundingBoxMin + boundingBoxMax) * 0.5f;
        radius = glm::length(boundingBoxMax - center);
    }
};