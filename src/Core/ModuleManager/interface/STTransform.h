#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "AYMathType.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

struct STTransform
{
    class Rotation {
    public:
        enum Type { EULER, QUAT };

        // 构造函数
        Rotation() : _type(EULER), _data(AYMath::Vector3(0.0f)) {}
        explicit Rotation(const AYMath::Vector3& euler) : _type(EULER), _data(euler) {}
        explicit Rotation(const AYMath::Quaternion& quat) : _type(QUAT), _data(quat) {}

        // 获取矩阵
        AYMath::Matrix4 getMatrix() const {
            if (_type == EULER) {
                return glm::eulerAngleXYZ(_data.euler.x, _data.euler.y, _data.euler.z);
            }
            else {
                return glm::mat4_cast(_data.quat);
            }
        }

        // 获取欧拉角（弧度）
        AYMath::Vector3 getEulerAngles() const {
            return (_type == EULER) ? _data.euler : glm::eulerAngles(_data.quat);
        }

        // 获取四元数
        AYMath::Quaternion getQuaternion() const {
            return (_type == EULER) ? AYMath::Quaternion(_data.euler) : _data.quat;
        }

        // 设置欧拉角（弧度）
        void setEulerAngles(const AYMath::Vector3& euler) {
            _type = EULER;
            _data.euler = euler;
        }

        // 设置四元数
        void setQuaternion(const AYMath::Quaternion& quat) {
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
                _data.quat = AYMath::Quaternion(_data.euler);
                _type = QUAT;
            }
        }

        Type getType() const { return _type; }

    private:
        Type _type;
        union Data {
            AYMath::Vector3 euler;
            AYMath::Quaternion quat;

            Data() : euler(0.0f) {}
            explicit Data(const AYMath::Vector3& e) : euler(e) {}
            explicit Data(const AYMath::Quaternion& q) : quat(q) {}
        } _data;
    };

public:
    STTransform(AYMath::Vector3 pos = AYMath::Vector3(0.0f),
        AYMath::Vector3 euler = AYMath::Vector3(0.0f),
        AYMath::Vector3 scl = AYMath::Vector3(1.0f))
        : position(pos), rotation(euler), scale(scl) {
    }

    AYMath::Vector3 position = AYMath::Vector3(0.0f);
    Rotation rotation;
    AYMath::Vector3 scale = AYMath::Vector3(1.0f);

    AYMath::Matrix4 getTransformMatrix() const {
        AYMath::Matrix4 matrix(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = matrix * rotation.getMatrix();
        matrix = glm::scale(matrix, scale);
        return matrix;
    }

    AYMath::Matrix4 getPixelSpaceMatrix(float pixelsPerMeter,
        const AYMath::Vector3& origin = AYMath::Vector3(0.5f),
        const AYMath::Vector3& additionalScale = AYMath::Vector3(1.0f)) const
    {
        //最终模型矩阵 = 缩放 -》 旋转 -》 平移 -》 单位 
        AYMath::Matrix4 matrix(1.0f);

        AYMath::Vector3 pixelPos = position * pixelsPerMeter;
        AYMath::Vector3 totalScale = scale * additionalScale;

        matrix = glm::translate(matrix, pixelPos);

        AYMath::Vector3 originOffset = AYMath::Vector3(origin.x, origin.y, origin.z) * totalScale;
        matrix = glm::translate(matrix, -originOffset);
        matrix = matrix * rotation.getMatrix();

        matrix = glm::scale(matrix, totalScale);
        return matrix;
    }

    // 获取方向向量
    AYMath::Vector3 getForwardVector() const 
    {
        return glm::normalize(AYMath::Vector3(rotation.getMatrix() * AYMath::Vector4(0.0f, 0.0f, 1.0f, 0.0f)));
    }

    AYMath::Vector3 getUpVector() const 
    {
        return glm::normalize(AYMath::Vector3(rotation.getMatrix() * AYMath::Vector4(0.0f, 1.0f, 0.0f, 0.0f)));
    }

    AYMath::Vector3 getRightVector() const 
    {
        return glm::normalize(AYMath::Vector3(rotation.getMatrix() * AYMath::Vector4(1.0f, 0.0f, 0.0f, 0.0f)));
    }

    static bool approximatelyEqual(const AYMath::Vector3& a, const AYMath::Vector3& b, float epsilon = glm::epsilon<float>())
    {
        return glm::all(glm::lessThanEqual(glm::abs(a - b), AYMath::Vector3(epsilon)));
    }

    bool operator==(const STTransform& other) const {
        return approximatelyEqual(position, other.position) &&
            approximatelyEqual(rotation.getEulerAngles(), other.rotation.getEulerAngles()) &&
            approximatelyEqual(scale, other.scale);
    }
};