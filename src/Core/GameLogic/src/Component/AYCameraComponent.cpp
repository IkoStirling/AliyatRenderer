#include "Component/AYCameraComponent.h"
#include "AYEntrant.h"
#include "AYRendererManager.h"
#include "BaseRendering/Camera/AYCamera2D.h"
#include "BaseRendering/Camera/AYCamera3D.h"
#include <random>

namespace {
    float _randomRange(float min, float max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        return static_cast<float>(dis(gen));
    }
}

namespace ayt::engine::game
{
    using namespace render;

    CameraComponent::CameraComponent(ICamera* camera) :
        _boundCamera(camera)
    {
        auto cameraSystem = GET_CAST_MODULE(RendererManager, "Renderer")->getCameraSystem();

        if (_boundCamera) {
            cameraSystem->addCamera(getOwner()->getName() + "_" + _name, _boundCamera);
        }
    }

    void CameraComponent::beginPlay()
    {
        if (!_boundCamera)
            setupCamera(ICamera::Type::PERSPECTIVE_3D);
        switch (_boundCamera->getType()) {
        case ICamera::Type::ORTHOGRAPHIC_2D:
            static_cast<Camera2D*>(_boundCamera)->setCurrentPosition(static_cast<Entrant*>(getOwner())->getPosition());
            break;
        case ICamera::Type::PERSPECTIVE_3D:
            break;
        }
    }

    void CameraComponent::update(float delta_time)
    {
        if (!_boundCamera || !getOwner()) return;

        // 获取所有者变换
        auto& trans = static_cast<Entrant*>(getOwner())->getTransform();

        // 根据相机类型更新
        switch (_boundCamera->getType()) {
        case ICamera::Type::ORTHOGRAPHIC_2D:
            _updateCamera2D(trans);
            break;
        case ICamera::Type::PERSPECTIVE_3D:
            _update3DCamera(trans);
            break;
        }
    }

    void CameraComponent::endPlay()
    {

    }

    void CameraComponent::bindCamera(ICamera* camera)
    {
        _boundCamera = camera;
    }

    void CameraComponent::activate()
    {
        auto cameraSystem = GET_CAST_MODULE(RendererManager, "Renderer")->getCameraSystem();
        cameraSystem->switchCamera(getOwner()->getName() + "_" + _name);
    }

    ICamera* CameraComponent::getCamera() const
    {
        return _boundCamera;
    }

    void CameraComponent::shake(float intensity, float duration)
    {
        _shakeIntensity = intensity;
        _shakeDuration = duration;
        _shakeTimer = 0.0f;
    }

    void CameraComponent::setZoom(float zoom)
    {
        if (!_boundCamera) return;
        _boundCamera->setZoom(zoom);
    }

    void CameraComponent::setupCamera(ICamera::Type type)
    {
        auto cameraSystem = GET_CAST_MODULE(RendererManager, "Renderer")->getCameraSystem();

        switch (type)
        {
        case ICamera::Type::PERSPECTIVE_3D:
            _boundCamera = cameraSystem->createCamera<Camera3D>(
                getOwner()->getName() + "_" + _name
            );
            break;
        case ICamera::Type::ORTHOGRAPHIC_2D:
            _boundCamera = cameraSystem->createCamera<Camera2D>(
                getOwner()->getName() + "_" + _name
            );
            break;
        case ICamera::Type::CUSTOM:
            break;
        default:
            break;
        }
    }

    void CameraComponent::_updateCamera2D(const math::Transform& ownerTrans)
    {
        auto* cam2D = dynamic_cast<Camera2D*>(_boundCamera);
        if (!cam2D) return;

        // 直接同步位置（可根据需求添加偏移）
        auto& pos = ownerTrans.position;
        cam2D->setTargetPosition(glm::vec2(pos.x, pos.y));
    }

    void CameraComponent::_update3DCamera(const math::Transform& ownerTrans)
    {
        auto* cam3D = dynamic_cast<Camera3D*>(_boundCamera);
        if (!cam3D) return;

        // 示例：第三人称跟随
        auto& pos = ownerTrans.position;
        glm::vec3 offset(0, 20.0f, -500.0f); // 后方上方偏移
        //glm::vec3 tp(pos + offset); // 后方上方偏移
        //std::cout << "[CameraComponent] camera target location [" << tp.x << ", " << tp.y << ", " << tp.z << "]\n";
        cam3D->setTargetPosition(pos + offset);
        cam3D->setLookAt(
            pos + offset,
            pos,
            glm::vec3(0, 1, 0)
        );
    }

    void CameraComponent::_applyShakeEffect(float delta)
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
}

