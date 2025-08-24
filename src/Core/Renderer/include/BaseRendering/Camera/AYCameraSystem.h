#pragma once
#include "IAYCamera.h"
#include <unordered_map>
#include <string>
#include <memory>

class AYCameraSystem
{
public:
    static const std::string SCREEN_SPACE_CAMERA;
public:
    AYCameraSystem();
    void shutdown();

    void update(float delta_time);
    // 相机管理
    template<typename T, typename... Args>
    T* createCamera(const std::string& name, Args&&... args);

    void addCamera(const std::string& name, IAYCamera* camera);

    void removeCamera(const std::string& name);

    void switchCamera(uint32_t cameraID);
    void switchCamera(const std::string& name);
    IAYCamera* getActiveCamera() const;
    uint32_t getActiveCameraID() const;
    uint32_t getCameraID(const std::string& name) const;
    IAYCamera* getCamera(uint32_t cameraID) const;
    IAYCamera* getCamera(const std::string& name) const;

    // 更改窗口大小后调用该函数，已嵌入回调
    void setViewportAll(const glm::vec4& viewport); 

private:
    std::unordered_map<uint32_t, std::unique_ptr<IAYCamera>> _cameras;
    std::unordered_map<std::string, uint32_t> _cameraMap;
    IAYCamera* _activeCamera = nullptr;
    uint32_t _currentIndex = 0; // 0代表当前没有有效相机
    uint32_t _maxIndex = 0;     // 0代表当前没有有效相机
};

template<typename T, typename... Args>
inline T* AYCameraSystem::createCamera(const std::string& name, Args&&... args)
{
    auto camera = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = camera.get();
    _cameras[++_maxIndex] = std::move(camera);
    _cameraMap[name] = _maxIndex;
    return ptr;
}
