#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

struct STTransform
{
    class Rotation {
    public:
        enum Type { EULER, QUAT };

        // 构造函数
        Rotation() : _type(EULER), _data(glm::vec3(0.0f)) {}
        explicit Rotation(const glm::vec3& euler) : _type(EULER), _data(euler) {}
        explicit Rotation(const glm::quat& quat) : _type(QUAT), _data(quat) {}

        // 获取矩阵
        glm::mat4 getMatrix() const {
            if (_type == EULER) {
                return glm::eulerAngleXYZ(_data.euler.x, _data.euler.y, _data.euler.z);
            }
            else {
                return glm::mat4_cast(_data.quat);
            }
        }

        // 获取欧拉角（弧度）
        glm::vec3 getEulerAngles() const {
            return (_type == EULER) ? _data.euler : glm::eulerAngles(_data.quat);
        }

        // 获取四元数
        glm::quat getQuaternion() const {
            return (_type == EULER) ? glm::quat(_data.euler) : _data.quat;
        }

        // 设置欧拉角（弧度）
        void setEulerAngles(const glm::vec3& euler) {
            _type = EULER;
            _data.euler = euler;
        }

        // 设置四元数
        void setQuaternion(const glm::quat& quat) {
            _type = QUAT;
            _data.quat = quat;
        }

        // 类型转换
        void switchToEuler() {
            if (_type == QUAT) {
                _data.euler = glm::eulerAngles(_data.quat);
                _type = EULER;
            }
        }

        void switchToQuat() {
            if (_type == EULER) {
                _data.quat = glm::quat(_data.euler);
                _type = QUAT;
            }
        }

        Type getType() const { return _type; }

    private:
        Type _type;
        union Data {
            glm::vec3 euler;
            glm::quat quat;

            Data() : euler(0.0f) {}
            explicit Data(const glm::vec3& e) : euler(e) {}
            explicit Data(const glm::quat& q) : quat(q) {}
        } _data;
    };

public:
    STTransform(glm::vec3 pos = glm::vec3(0.0f),
        glm::vec3 euler = glm::vec3(0.0f),
        glm::vec3 scl = glm::vec3(1.0f))
        : position(pos), rotation(euler), scale(scl) {
    }

    glm::vec3 position = glm::vec3(0.0f);
    Rotation rotation;
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 getTransformMatrix() const {
        glm::mat4 matrix(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = matrix * rotation.getMatrix();
        matrix = glm::scale(matrix, scale);
        return matrix;
    }

    glm::mat4 getPixelSpaceMatrix(float pixelsPerMeter,
        const glm::vec3& origin = glm::vec3(0.5f),
        const glm::vec3& additionalScale = glm::vec3(1.0f)) const
    {
        //最终模型矩阵 = 缩放 -》 旋转 -》 平移 -》 单位 
        glm::mat4 matrix(1.0f);

        glm::vec3 pixelPos = position * pixelsPerMeter;
        glm::vec3 totalScale = scale * additionalScale;

        matrix = glm::translate(matrix, pixelPos);

        glm::vec3 originOffset = glm::vec3(origin.x, origin.y, origin.z) * totalScale;
        matrix = glm::translate(matrix, -originOffset);
        matrix = matrix * rotation.getMatrix();

        matrix = glm::scale(matrix, totalScale);
        return matrix;
    }

    // 获取方向向量
    glm::vec3 getForwardVector() const {
        return glm::normalize(glm::vec3(rotation.getMatrix() * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 getUpVector() const {
        return glm::normalize(glm::vec3(rotation.getMatrix() * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
    }

    glm::vec3 getRightVector() const {
        return glm::normalize(glm::vec3(rotation.getMatrix() * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
    }

    bool approximatelyEqual(const glm::vec3& a, const glm::vec3& b, float epsilon = glm::epsilon<float>()) const {
        return glm::all(glm::lessThanEqual(glm::abs(a - b), glm::vec3(epsilon)));
    }

    bool operator==(const STTransform& other) const {
        return approximatelyEqual(position, other.position) &&
            approximatelyEqual(rotation.getEulerAngles(), other.rotation.getEulerAngles()) &&
            approximatelyEqual(scale, other.scale);
    }
};