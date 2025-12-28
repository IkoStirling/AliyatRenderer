#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "AYMathType.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

namespace ayt::engine::math
{
    struct Transform
    {
        class Rotation {
        public:
            enum Type { EULER, QUAT };

            // 构造函数
            Rotation() : _type(EULER), _data(math::Vector3(0.0f)) {}
            explicit Rotation(const math::Vector3& euler) : _type(EULER), _data(euler) {}
            explicit Rotation(const math::Quaternion& quat) : _type(QUAT), _data(quat) {}

            // 获取矩阵
            math::Matrix4 getMatrix() const {
                if (_type == EULER) {
                    return glm::eulerAngleXYZ(_data.euler.x, _data.euler.y, _data.euler.z);
                }
                else {
                    return glm::mat4_cast(_data.quat);
                }
            }

            // 获取欧拉角（弧度）
            math::Vector3 getEulerAngles() const {
                return (_type == EULER) ? _data.euler : glm::eulerAngles(_data.quat);
            }

            // 获取四元数
            math::Quaternion getQuaternion() const {
                return (_type == EULER) ? math::Quaternion(_data.euler) : _data.quat;
            }

            // 设置欧拉角（弧度）
            void setEulerAngles(const math::Vector3& euler) {
                _type = EULER;
                _data.euler = euler;
            }

            // 设置四元数
            void setQuaternion(const math::Quaternion& quat) {
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
                    _data.quat = math::Quaternion(_data.euler);
                    _type = QUAT;
                }
            }

            Type getType() const { return _type; }

        private:
            Type _type;
            union Data {
                math::Vector3 euler;
                math::Quaternion quat;

                Data() : euler(0.0f) {}
                explicit Data(const math::Vector3& e) : euler(e) {}
                explicit Data(const math::Quaternion& q) : quat(q) {}
            } _data;
        };

    public:
        Transform(math::Vector3 pos = math::Vector3(0.0f),
            math::Vector3 euler = math::Vector3(0.0f),
            math::Vector3 scl = math::Vector3(1.0f))
            : position(pos), rotation(euler), scale(scl) {
        }

        math::Vector3 position = math::Vector3(0.0f);
        Rotation rotation;
        math::Vector3 scale = math::Vector3(1.0f);

        math::Matrix4 getTransformMatrix() const {
            math::Matrix4 matrix(1.0f);
            matrix = glm::translate(matrix, position);
            matrix = matrix * rotation.getMatrix();
            matrix = glm::scale(matrix, scale);
            return matrix;
        }

        math::Matrix4 getPixelSpaceMatrix(float pixelsPerMeter,
            const math::Vector3& origin = math::Vector3(0.5f),
            const math::Vector3& additionalScale = math::Vector3(1.0f)) const
        {
            //最终模型矩阵 = 缩放 -》 旋转 -》 平移 -》 单位 
            math::Matrix4 matrix(1.0f);

            math::Vector3 pixelPos = position * pixelsPerMeter;
            math::Vector3 totalScale = scale * additionalScale;

            matrix = glm::translate(matrix, pixelPos);

            math::Vector3 originOffset = math::Vector3(origin.x, origin.y, origin.z) * totalScale;
            matrix = glm::translate(matrix, -originOffset);
            matrix = matrix * rotation.getMatrix();

            matrix = glm::scale(matrix, totalScale);
            return matrix;
        }

        // 获取方向向量
        math::Vector3 getForwardVector() const
        {
            return glm::normalize(math::Vector3(rotation.getMatrix() * math::Vector4(0.0f, 0.0f, 1.0f, 0.0f)));
        }

        math::Vector3 getUpVector() const
        {
            return glm::normalize(math::Vector3(rotation.getMatrix() * math::Vector4(0.0f, 1.0f, 0.0f, 0.0f)));
        }

        math::Vector3 getRightVector() const
        {
            return glm::normalize(math::Vector3(rotation.getMatrix() * math::Vector4(1.0f, 0.0f, 0.0f, 0.0f)));
        }

        static bool approximatelyEqual(const math::Vector3& a, const math::Vector3& b, float epsilon = glm::epsilon<float>())
        {
            return glm::all(glm::lessThanEqual(glm::abs(a - b), math::Vector3(epsilon)));
        }

        bool operator==(const Transform& other) const {
            return approximatelyEqual(position, other.position) &&
                approximatelyEqual(rotation.getEulerAngles(), other.rotation.getEulerAngles()) &&
                approximatelyEqual(scale, other.scale);
        }
    };

}