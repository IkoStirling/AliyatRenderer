#include "BaseRendering/Camera/AYCameraSystem.h"
#include "BaseRendering/Camera/AY3DCamera.h"

const std::string AYCameraSystem::SCREEN_SPACE_CAMERA = "default";

AYCameraSystem::AYCameraSystem()
{
    createCamera<IAYCamera>(SCREEN_SPACE_CAMERA);
    switchCamera(SCREEN_SPACE_CAMERA);
}

void AYCameraSystem::shutdown()
{
    _cameras.clear();
    _cameraMap.clear();
}

void AYCameraSystem::update(float delta_time)
{
    for (auto& [name, camera] : _cameras)
    {
        camera->update(delta_time);
    } 
    _activeCamera->onCameraMoved();
}

void AYCameraSystem::addCamera(const std::string& name, IAYCamera* camera)
{
    _cameras[++_maxIndex] = std::unique_ptr<IAYCamera>(camera);
    _cameraMap[name] = _maxIndex;
}


void AYCameraSystem::removeCamera(const std::string& name)
{
    auto id = _cameraMap[name];
    if (id)
    {
        if (auto it = _cameras.find(id); it != _cameras.end())
        {
            _cameras.erase(it);
        }
        _cameraMap[name] = 0;
    }
}

void AYCameraSystem::switchCamera(uint32_t cameraID)
{
    if (auto it = _cameras.find(cameraID); it != _cameras.end()) {
        _activeCamera = it->second.get();
        _currentIndex = cameraID;
    }
}

void AYCameraSystem::switchCamera(const std::string& name)
{
    // c++17写法
    if (auto it = _cameraMap.find(name); it != _cameraMap.end()) {
        switchCamera(it->second);
    }
}

void AYCameraSystem::setViewportAll(const glm::vec4& viewport)
{
    for (auto it = _cameras.begin(); it != _cameras.end(); it++) {
        it->second.get()->setViewport(viewport);
    }
}

IAYCamera* AYCameraSystem::getActiveCamera() const 
{
    return _activeCamera;
}

uint32_t AYCameraSystem::getActiveCameraID() const
{
    return _currentIndex;
}

uint32_t AYCameraSystem::getCameraID(const std::string& name) const
{
    if (auto it = _cameraMap.find(name); it != _cameraMap.end()) {
        return it->second;
    }
    return 0;
}

IAYCamera* AYCameraSystem::getCamera(uint32_t cameraID) const
{
    if (auto it = _cameras.find(cameraID); it != _cameras.end()) {
        return it->second.get();
    }
    return nullptr;
}

IAYCamera* AYCameraSystem::getCamera(const std::string& name) const
{
    if (auto it = _cameraMap.find(name); it != _cameraMap.end()) {
        return getCamera(it->second);
    }
    return nullptr;
}