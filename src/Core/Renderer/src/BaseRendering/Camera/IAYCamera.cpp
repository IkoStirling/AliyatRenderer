#include "BaseRendering/Camera/IAYCamera.h"
#include "AYLogger.h"
#include "AYEventRegistry.h"
#include "Event_CameraMove.h"

namespace ayt::engine::render
{
    using namespace ::ayt::engine::event;

	math::Matrix4 IAYCamera::getViewMatrix() const
	{
		return math::Matrix4(1.f);
	}

	math::Matrix4 IAYCamera::getProjectionMatrix() const
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

    void IAYCamera::update(float delta_time)
    {

    }

    IAYCamera::Type IAYCamera::getType() const
    {
        return Type::DEFAULT_SCREEN;
    }

    void IAYCamera::setViewport(const math::Vector4& viewport)
    {
        _dirtyView = true;
        _dirtyProjection = true;
        _viewport = viewport;
    }

    math::Vector4 IAYCamera::getViewport() const
    {
        return _viewport;
    }

    void IAYCamera::setZoom(float zoom) {
        _dirtyView = true;
        _dirtyProjection = true;
        _zoom = glm::clamp(zoom, 0.1f, 10.0f);
        AYLOG_INFO("[IAYCamera] zoom: {}", _zoom);
    }

    float IAYCamera::getPixelPerMeter() const { return 66.7f; }

    void IAYCamera::setAdditionalOffset(const math::Vector2& offset) { _additionalOffset = offset; }

    const math::Vector3 IAYCamera::getPosition() const { return _transform.position / getPixelPerMeter(); }

    void IAYCamera::onCameraMoved() const
    {
        if (_lastTransform == _transform)
            return;
        EventRegistry::publish(Event_CameraMove::staticGetType(),
            [this](IEvent* event) {
                auto eI = static_cast<Event_CameraMove*>(event);
                eI->transform = _transform;
            });
    }
}