#pragma once
#include "STTransform.h"

namespace ayt::engine::render
{
    class ICamera {
    public:
        virtual ~ICamera() = default;

        // 获取视图矩阵（View Matrix）
        virtual math::Matrix4 getViewMatrix() const;

        // 获取投影矩阵（Projection Matrix）
        virtual math::Matrix4 getProjectionMatrix() const;

        // 每帧更新（参数可扩展）
        virtual void update(float delta_time);

        // 相机类型标识
        enum class Type { DEFAULT_SCREEN, PERSPECTIVE_3D, ORTHOGRAPHIC_2D, CUSTOM };
        virtual Type getType() const;
        
        // 视口控制
        virtual void setViewport(const math::Vector4& viewport);
        
        //由摄像机决定渲染视口
        math::Vector4 getViewport() const;

        virtual void setZoom(float zoom);

        virtual float getPixelPerMeter() const;

        virtual void setAdditionalOffset(const math::Vector2& offset);

        virtual const math::Vector3 getPosition() const;

        virtual void onCameraMoved() const;
    protected:
        math::Transform _transform;
        math::Transform _lastTransform;
        math::Vector3 _targetPosition = math::Vector3(0.f);
        math::Vector4 _viewport{ 0, 0, 1920, 1080 }; // x,y,width,height
        math::Vector2 _additionalOffset{ 0.0f };

        float _zoom = 1.f;

        mutable bool _dirtyView = true;
        mutable bool _dirtyProjection = true;
        mutable math::Matrix4 _cachedView = math::Matrix4(1.0);
        mutable math::Matrix4 _cachedProjection = math::Matrix4(1.0);
    };
}