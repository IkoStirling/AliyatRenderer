#include "Component/AYCameraComponent.h"
#include "AYEntrant.h"
#include "AYRendererManager.h"
#include "BaseRendering/Camera/AY2DCamera.h"
#include "BaseRendering/Camera/AY3DCamera.h"
#include <random>

namespace {
    float _randomRange(float min, float max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        return static_cast<float>(dis(gen));
    }
}

AYCameraComponent::AYCameraComponent(IAYCamera* camera) :
    _boundCamera(camera)
{
    auto cameraSystem = GET_CAST_MODULE(AYRendererManager, "Renderer")->getCameraSystem();

    if (_boundCamera) {
        cameraSystem->addCamera(getOwner()->getName() + "_" + _name, _boundCamera);
    }
}

void AYCameraComponent::beginPlay()
{
    if (!_boundCamera)
        setupCamera(IAYCamera::Type::PERSPECTIVE_3D);
    switch (_boundCamera->getType()) {
    case IAYCamera::Type::ORTHOGRAPHIC_2D:
        static_cast<AY2DCamera*>(_boundCamera)->setCurrentPosition(static_cast<AYEntrant*>(getOwner())->getPosition());
        break;
    case IAYCamera::Type::PERSPECTIVE_3D:
        break;
    }
}

void AYCameraComponent::update(float delta_time)
{
    if (!_boundCamera || !getOwner()) return;

    // 获取所有者变换
    auto& trans = static_cast<AYEntrant*>(getOwner())->getTransform();

    // 根据相机类型更新
    switch (_boundCamera->getType()) {
    case IAYCamera::Type::ORTHOGRAPHIC_2D:
        _update2DCamera(trans);
        break;
    case IAYCamera::Type::PERSPECTIVE_3D:
        _update3DCamera(trans);
        break;
    }
}

void AYCameraComponent::endPlay()
{
    if (_boundCamera) {
        auto cameraSystem = GET_CAST_MODULE(AYRendererManager, "Renderer")->getCameraSystem();

        cameraSystem->removeCamera(_boundCamera);
    }
}

void AYCameraComponent::bindCamera(IAYCamera* camera)
{
    _boundCamera = camera;
}

void AYCameraComponent::activate()
{
    auto cameraSystem = GET_CAST_MODULE(AYRendererManager, "Renderer")->getCameraSystem();
    cameraSystem->switchCamera(getOwner()->getName() + "_" + _name);
}

IAYCamera* AYCameraComponent::getCamera() const
{
    return _boundCamera;
}

void AYCameraComponent::shake(float intensity, float duration)
{
    _shakeIntensity = intensity;
    _shakeDuration = duration;
    _shakeTimer = 0.0f;
}

void AYCameraComponent::setupCamera(IAYCamera::Type type)
{
    auto cameraSystem = GET_CAST_MODULE(AYRendererManager, "Renderer")->getCameraSystem();

    switch (type)
    {
    case IAYCamera::Type::PERSPECTIVE_3D:
        _boundCamera = cameraSystem->createCamera<AY3DCamera>(
            getOwner()->getName() + "_" + _name
        );
        break;
    case IAYCamera::Type::ORTHOGRAPHIC_2D:
        _boundCamera = cameraSystem->createCamera<AY2DCamera>(
            getOwner()->getName() + "_" + _name
        );
        break;
    case IAYCamera::Type::CUSTOM:
        break;
    default:
        break;
    }
}

void AYCameraComponent::_update2DCamera(const STTransform& ownerTrans)
{
    auto* cam2D = dynamic_cast<AY2DCamera*>(_boundCamera);
    if (!cam2D) return;

    // 直接同步位置（可根据需求添加偏移）
    auto& pos = ownerTrans.position;
    cam2D->setTargetPosition(glm::vec2(pos.x, pos.y));
}

void AYCameraComponent::_update3DCamera(const STTransform& ownerTrans)
{
    auto* cam3D = dynamic_cast<AY3DCamera*>(_boundCamera);
    if (!cam3D) return;

    // 示例：第三人称跟随
    auto& pos = ownerTrans.position;
    glm::vec3 offset(0, 20.0f, -500.0f); // 后方上方偏移
    cam3D->setTargetPosition(pos + offset);
    cam3D->setLookAt(
        pos + offset,
        pos,
        glm::vec3(0, 1, 0)
    );
}

void AYCameraComponent::_applyShakeEffect(float delta)
{
    _shakeTimer += delta;
    float progress = _shakeTimer / _shakeDuration;
    float currentIntensity = _shakeIntensity * (1.0f - progress);

    glm::vec3 randomOffset(
        _randomRange(-currentIntensity, currentIntensity),
        _randomRange(-currentIntensity, currentIntensity),
        0
    );

    _boundCamera->setAdditionalOffset(randomOffset);
}


