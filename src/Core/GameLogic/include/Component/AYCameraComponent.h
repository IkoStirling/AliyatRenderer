#pragma once
#include "IAYComponent.h"
#include "BaseRendering/Camera/IAYCamera.h"
#include "AYEntrant.h"

namespace ayt::engine::game
{
    using ::ayt::engine::render::ICamera;

    class CameraComponent : public IComponent
    {
    public:
        CameraComponent(ICamera* camera = nullptr);

        void beginPlay() override;

        void update(float delta_time) override;

        void endPlay() override;

        void bindCamera(ICamera* camera);

        void activate();

        ICamera* getCamera() const;

        void shake(float intensity, float duration);

        void setZoom(float zoom);

        void setupCamera(ICamera::Type type);

    private:
        void _updateCamera2D(const math::Transform& ownerTrans);

        void _update3DCamera(const math::Transform& ownerTrans);

        void _applyShakeEffect(float delta);

        ICamera* _boundCamera = nullptr;

        float _shakeIntensity = 0.0f;
        float _shakeDuration = 0.0f;
        float _shakeTimer = 0.0f;
    };
}