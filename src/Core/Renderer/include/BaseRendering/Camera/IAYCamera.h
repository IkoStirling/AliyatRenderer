#pragma once
#include <glm/glm.hpp>
#include "STTransform.h"
#include "AYEventRegistry.h"
#include "Event_CameraMove.h"

class IAYCamera {
public:
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

    virtual const glm::vec3 getPosition() const { return _transform.position; }

    virtual void onCameraMoved() const
    {
        if (_lastTransform == _transform)
            return;
        AYEventRegistry::publish(Event_CameraMove::staticGetType(),
            [this](IAYEvent* event) {
                auto eI = static_cast<Event_CameraMove*>(event);
                eI->transform = _transform;
            });
    }
protected:
    STTransform _transform;
    STTransform _lastTransform;
    glm::vec3 _targetPosition = glm::vec3(0.f);
    glm::vec4 _viewport{ 0, 0, 1920, 1080 }; // x,y,width,height
    glm::vec2 _additionalOffset{ 0.0f };
};