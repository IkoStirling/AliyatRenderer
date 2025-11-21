#pragma once
#include "BaseRendering/Camera/AY2DCamera.h"
#include "AYRendererManager.h"

void AY2DCamera::update(float delta_time)
{
    _lastTransform = _transform;

    // 计算实际视口大小(世界单位/米)
    float ppm = getPixelPerMeter();
    float viewWidth = _viewport.z / (ppm * _zoom);  // 视口宽度(米)
    float viewHeight = _viewport.w / (ppm * _zoom); // 视口高度(米)

    // 2. 计算死区边界（物理单位-米）
    AYMath::Vector2 deadzoneMin(
        -viewWidth * 0.5f + viewWidth * _deadzone.x,  // 左边界
        -viewHeight * 0.5f + viewHeight * _deadzone.z // 下边界
    );
    AYMath::Vector2 deadzoneMax(
        viewWidth * 0.5f - viewWidth * (1 - _deadzone.y),   // 右边界
        viewHeight * 0.5f - viewHeight * (1 - _deadzone.w)  // 上边界
    );

    // 计算目标点相对于摄像机中心的偏移(世界单位)
    AYMath::Vector2 targetOffset = AYMath::Vector2(_targetPosition) - AYMath::Vector2(_transform.position);

    // 3. 计算移动偏移（物理单位-米）
    AYMath::Vector2 moveOffset(0.0f);

    if (targetOffset.x < deadzoneMin.x) {
        moveOffset.x = targetOffset.x - deadzoneMin.x;
    }
    else if (targetOffset.x > deadzoneMax.x) {
        moveOffset.x = targetOffset.x - deadzoneMax.x;
    }

    if (targetOffset.y < deadzoneMin.y) {
        moveOffset.y = targetOffset.y - deadzoneMin.y;
    }
    else if (targetOffset.y > deadzoneMax.y) {
        moveOffset.y = targetOffset.y - deadzoneMax.y;
    }

    // 应用移动（物理单位-米）
    if (glm::length(moveOffset) > 0) _dirtyView = true;

    AYMath::Vector2 newPos = AYMath::Vector2(_transform.position) + moveOffset * _moveSpeed * delta_time;

    // 地图边界约束（物理单位-米）
    newPos.x = glm::clamp(newPos.x,
        _mapBounds.x + viewWidth * 0.5f,
        _mapBounds.y - viewWidth * 0.5f);
    newPos.y = glm::clamp(newPos.y,
        _mapBounds.z + viewHeight * 0.5f,
        _mapBounds.w - viewHeight * 0.5f);

    _transform.position = AYMath::Vector3(newPos, _transform.position.z);
}

AYMath::Matrix4 AY2DCamera::getViewMatrix() const
{
    if (_dirtyView)
    {
        // 将屏幕中心定义为(0,0), 正常是在左上角, 不进行缩放
        _cachedView = glm::translate(AYMath::Matrix4(1.0f),
            -AYMath::Vector3(
                AYMath::Vector2(_transform.position) + _additionalOffset,
                0.0f));
        _dirtyView = false;
    }
    return _cachedView;
}

AYMath::Matrix4 AY2DCamera::getProjectionMatrix() const
{
    if (_dirtyProjection)
    {
        // 返回物理比例下的投影矩阵
        float ppm = getPixelPerMeter();
        float zoomedWidth = _viewport.z / (_zoom * ppm);
        float zoomedHeight = _viewport.w / (_zoom * ppm);

        _cachedProjection = glm::ortho(
            -zoomedWidth * 0.5f,  // left
            zoomedWidth * 0.5f,   // right
            -zoomedHeight * 0.5f,  // bottom 
            zoomedHeight * 0.5f,   // top
            _near,
            _far
        );
        _dirtyProjection = false;
    }
    return _cachedProjection;
}

void AY2DCamera::setViewBox(float view_near, float view_far)
{
    _dirtyProjection = true;
    _near = view_near;
    _far = view_far;
}

void AY2DCamera::setDeadzone(const AYMath::Vector4& zone)
{
    _deadzone = zone;
} 

void AY2DCamera::setTargetPosition(const AYMath::Vector2& targetPos)
{
    _targetPosition = AYMath::Vector3(targetPos, 0.f);
}

void AY2DCamera::setCurrentPosition(const AYMath::Vector2& currentPos)
{
    _transform.position = AYMath::Vector3(currentPos, 0.f);
}

void AY2DCamera::showDeadzone(bool switcher)
{
    auto rendererManager = GET_CAST_MODULE(AYRendererManager, "Renderer");
    if (switcher && !_ddeadzone)
    {
        _ddeadzone = rendererManager->addDebugDraw(false, [](AYRenderer* renderer, AYRenderDevice* device) {
            renderer->getCoreRenderer()
                ->drawRect2D(
                    { AYMath::Vector3(1920 * 0.5f, 1080 * 0.5f, 0) },
                    AYMath::Vector2(1920 * 0.4f, 1080 * 0.4f),
                    0,
                    true,
                    AYCoreRenderer::Space::Screen);
            });
    }
    else if (!switcher && _ddeadzone)
    {
        rendererManager->removeDebugDraw(_ddeadzone);
    }

}

