#pragma once
#include <glm/glm.hpp>
#include "STTransform.h"
#include "AYEventRegistry.h"
#include "Event_CameraMove.h"
#include <spdlog/spdlog.h>

class IAYCamera {
public:
    virtual ~IAYCamera() = default;

    // 获取视图矩阵（View Matrix）
    virtual glm::mat4 getViewMatrix() const
    {
        return glm::mat4(1.f);
    }

    // 获取投影矩阵（Projection Matrix）
    virtual glm::mat4 getProjectionMatrix() const
    {
        if (_dirtyProjection)
        {
            // 返回物理比例下的投影矩阵
            float ppm = getPixelPerMeter();
            ppm = 1;
            float width = _viewport.z / ppm;
            float height = _viewport.w / ppm;

            _cachedProjection = glm::ortho(
                0.0f,  // left
                width,   // right
                height,  // bottom 
                0.0f,   // top
                -1.0f,
                1.0f
            );
            _dirtyProjection = false;
        }
        return _cachedProjection;
    }

    // 每帧更新（参数可扩展）
    virtual void update(float delta_time)
    {

    }

    // 相机类型标识
    enum class Type { DEFAULT_SCREEN, PERSPECTIVE_3D, ORTHOGRAPHIC_2D, CUSTOM };
    virtual Type getType() const
    {
        return Type::DEFAULT_SCREEN;
    }

    // 视口控制
    virtual void setViewport(const glm::vec4& viewport) 
    { 
        _dirtyView = true;
        _dirtyProjection = true;
        _viewport = viewport; 
    }   //由摄像机决定渲染视口
    glm::vec4 getViewport() const { return _viewport; }

    virtual void setZoom(float zoom) { 
        _dirtyView = true;
        _dirtyProjection = true;
        _zoom = glm::clamp(zoom, 0.1f, 10.0f);
        spdlog::info("[IAYCamera] zoom: {}", _zoom);
    }
    virtual float getPixelPerMeter() const { return 66.7f; }

    virtual void setAdditionalOffset(const glm::vec2& offset) { _additionalOffset = offset; }

    virtual const glm::vec3 getPosition() const { return _transform.position / getPixelPerMeter(); }

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

    float _zoom = 1.f;

    mutable bool _dirtyView = true;
    mutable bool _dirtyProjection = true;
    mutable glm::mat4 _cachedView = glm::mat4(1.0);
    mutable glm::mat4 _cachedProjection = glm::mat4(1.0);
};