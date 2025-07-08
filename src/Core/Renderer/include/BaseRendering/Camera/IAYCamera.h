#pragma once
#include <glm/glm.hpp>


class IAYCamera {
public:
    struct OrthoInfo
    {
        float left = 0.f;
        float right = 1920.f;
        float bottom = 0.f;
        float top = 1080.f;
        float near = -100.f;
        float far = 100.f;
    };

    virtual ~IAYCamera() = default;

    // 获取视图矩阵（View Matrix）
    virtual glm::mat4 getViewMatrix() const = 0;

    // 获取投影矩阵（Projection Matrix）
    virtual glm::mat4 getProjectionMatrix() const = 0;

    // 每帧更新（参数可扩展）
    virtual void update(float delta_time) = 0;

    // 相机类型标识
    enum class Type { PERSPECTIVE_3D, ORTHOGRAPHIC_2D, CUSTOM };
    virtual Type getType() const = 0;

    // 视口控制
    virtual void setViewport(const glm::vec4& viewport) { _viewport = viewport; }   //由摄像机决定渲染视口
    glm::vec4 getViewport() const { return _viewport; }

    virtual void setAdditionalOffset(const glm::vec2& offset) { _additionalOffset = offset; }

protected:
    glm::vec4 _viewport{ 0, 0, 1920, 1080 }; // x,y,width,height
    glm::vec2 _additionalOffset{ 0.0f };
};