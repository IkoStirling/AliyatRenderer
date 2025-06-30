#include "BaseRendering/Camera/AYCameraSystem.h"
#include "BaseRendering/Camera/AY3DCamera.h"

AYCameraSystem::AYCameraSystem()
{
    createCamera<AY3DCamera>("default");
    switchCamera("default");
}

void AYCameraSystem::addCamera(const std::string& name, IAYCamera* camera)
{
    _cameras[name] = std::unique_ptr<IAYCamera>(camera);
}

void AYCameraSystem::removeCamera(IAYCamera* camera) {
    for (auto it = _cameras.begin(); it != _cameras.end(); ) {
        if (it->second.get() == camera) {
            it = _cameras.erase(it);
        }
        else {
            ++it;
        }
    }
}

void AYCameraSystem::switchCamera(const std::string& name) {
    if (auto it = _cameras.find(name); it != _cameras.end()) {
        _activeCamera = it->second.get();
    }
}

IAYCamera* AYCameraSystem::getActiveCamera() const {
    return _activeCamera;
}


