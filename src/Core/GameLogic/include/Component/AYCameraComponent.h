#pragma once
#include "IAYComponent.h"
#include "BaseRendering/Camera/IAYCamera.h"
#include "AYEntrant.h"

class AYCameraComponent : public IAYComponent 
{
public:
    AYCameraComponent(IAYCamera* camera = nullptr);

    void beginPlay() override;

    void update(float delta_time) override;

    void endPlay() override;

    void bindCamera(IAYCamera* camera);

    void active();

    IAYCamera* getCamera() const;

    void shake(float intensity, float duration);

    void setupCamera(IAYCamera::Type type);

private:
    void _update2DCamera(const STTransform& ownerTrans);

    void _update3DCamera(const STTransform& ownerTrans);

    void _applyShakeEffect(float delta);

    IAYCamera* _boundCamera = nullptr;

    float _shakeIntensity = 0.0f;
    float _shakeDuration = 0.0f;
    float _shakeTimer = 0.0f;
};