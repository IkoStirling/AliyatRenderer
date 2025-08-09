#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct STTransform
{
    glm::vec3 position = glm::vec3(0.f);
    glm::vec3 rotation = glm::vec3(0.f);
    glm::vec3 scale = { 1,1,1 };

    glm::mat4 getTransformMatrix() const {
        glm::mat4 matrix(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        matrix = glm::scale(matrix, scale);
        return matrix;
    }

    // 获取前向向量（Z轴正方向）
    glm::vec3 getForwardVector() const {
        glm::mat4 matrix = getTransformMatrix();
        glm::vec3 forward = glm::vec3(matrix[2]);  // 第3列（Z轴）
        return glm::normalize(forward);            // 归一化去除缩放影响
    }

    // 获取上方向量（Y轴正方向）
    glm::vec3 getUpVector() const {
        glm::mat4 matrix = getTransformMatrix();
        glm::vec3 up = glm::vec3(matrix[1]);       // 第2列（Y轴）
        return glm::normalize(up);                 // 归一化去除缩放影响
    }

    // 获取右方向量（X轴正方向，可选）
    glm::vec3 getRightVector() const {
        glm::mat4 matrix = getTransformMatrix();
        glm::vec3 right = glm::vec3(matrix[0]);    // 第1列（X轴）
        return glm::normalize(right);              // 归一化去除缩放影响
    }

    bool approximatelyEqual(const glm::vec3& a, const glm::vec3& b, float epsilon = glm::epsilon<float>()) const {
        return glm::all(glm::lessThanEqual(glm::abs(a - b), glm::vec3(epsilon)));
    }

    bool operator==(const STTransform& other) const {
        return approximatelyEqual(position, other.position) &&
            approximatelyEqual(rotation, other.rotation) &&
            approximatelyEqual(scale, other.scale);
    }


};