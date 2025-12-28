#include "BaseRendering/Camera/AYCameraSystem.h"
#include "BaseRendering/Camera/AYCamera3D.h"
namespace ayt::engine::render
{
    const std::string CameraSystem::SCREEN_SPACE_CAMERA = "default";

    CameraSystem::CameraSystem()
    {
        createCamera<ICamera>(SCREEN_SPACE_CAMERA);
        switchCamera(SCREEN_SPACE_CAMERA);
    }

    void CameraSystem::shutdown()
    {
        _cameras.clear();
        _cameraMap.clear();
    }

    void CameraSystem::update(float delta_time)
    {
        for (auto& [name, camera] : _cameras)
        {
            camera->update(delta_time);
        }
        _activeCamera->onCameraMoved();
    }

    void CameraSystem::addCamera(const std::string& name, ICamera* camera)
    {
        _cameras[++_maxIndex] = std::unique_ptr<ICamera>(camera);
        _cameraMap[name] = _maxIndex;
    }


    void CameraSystem::removeCamera(const std::string& name)
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

    void CameraSystem::switchCamera(uint32_t cameraID)
    {
        if (auto it = _cameras.find(cameraID); it != _cameras.end()) {
            _activeCamera = it->second.get();
            _currentIndex = cameraID;
        }
    }

    void CameraSystem::switchCamera(const std::string& name)
    {
        // c++17写法
        if (auto it = _cameraMap.find(name); it != _cameraMap.end()) {
            switchCamera(it->second);
        }
    }

    void CameraSystem::setViewportAll(const glm::vec4& viewport)
    {
        for (auto it = _cameras.begin(); it != _cameras.end(); it++) {
            it->second.get()->setViewport(viewport);
        }
    }

    ICamera* CameraSystem::getActiveCamera() const
    {
        return _activeCamera;
    }

    uint32_t CameraSystem::getActiveCameraID() const
    {
        return _currentIndex;
    }

    uint32_t CameraSystem::getCameraID(const std::string& name) const
    {
        if (auto it = _cameraMap.find(name); it != _cameraMap.end()) {
            return it->second;
        }
        return 0;
    }

    ICamera* CameraSystem::getCamera(uint32_t cameraID) const
    {
        if (auto it = _cameras.find(cameraID); it != _cameras.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    ICamera* CameraSystem::getCamera(const std::string& name) const
    {
        if (auto it = _cameraMap.find(name); it != _cameraMap.end()) {
            return getCamera(it->second);
        }
        return nullptr;
    }
}