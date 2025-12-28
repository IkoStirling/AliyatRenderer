#pragma once
#include "IAYComponent.h"
#include "BaseRendering/Camera/IAYCamera.h"
#include "AYEntrant.h"

namespace ayt::engine::game
{
    using ::ayt::engine::render::IAYCamera;

    class AYCameraComponent : public IAYComponent
    {
    public:
        AYCameraComponent(IAYCamera* camera = nullptr);

        void beginPlay() override;

        void update(float delta_time) override;

        void endPlay() override;

        void bindCamera(IAYCamera* camera);

        void activate();

        IAYCamera* getCamera() const;

        void shake(float intensity, float duration);

        void setZoom(float zoom);

        void setupCamera(IAYCamera::Type type);

    private:
        void _update2DCamera(const math::Transform& ownerTrans);

        void _update3DCamera(const math::Transform& ownerTrans);

        void _applyShakeEffect(float delta);

        IAYCamera* _boundCamera = nullptr;

        float _shakeIntensity = 0.0f;
        float _shakeDuration = 0.0f;
        float _shakeTimer = 0.0f;
    };
}