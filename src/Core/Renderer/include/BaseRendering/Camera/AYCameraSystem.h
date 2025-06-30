#pragma once
#include "IAYCamera.h"
#include <unordered_map>
#include <string>
#include <memory>

class AYCameraSystem
{
public:
    AYCameraSystem();

    // 相机管理
    template<typename T, typename... Args>
    T* createCamera(const std::string& name, Args&&... args);

    void addCamera(const std::string& name, IAYCamera* camera);

    void removeCamera(IAYCamera* camera);

    void switchCamera(const std::string& name);

    IAYCamera* getActiveCamera() const;

private:
    std::unordered_map<std::string, std::unique_ptr<IAYCamera>> _cameras;
    IAYCamera* _activeCamera = nullptr;
};

template<typename T, typename... Args>
inline T* AYCameraSystem::createCamera(const std::string& name, Args&&... args)
{
    auto camera = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = camera.get();
    _cameras[name] = std::move(camera);
    return ptr;
}
