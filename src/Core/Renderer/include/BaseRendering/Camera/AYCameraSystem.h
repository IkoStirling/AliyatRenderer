#pragma once
#include "IAYCamera.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace ayt::engine::render
{
    class CameraSystem
    {
    public:
        static const std::string SCREEN_SPACE_CAMERA;
    public:
        CameraSystem();
        void shutdown();

        void update(float delta_time);
        // 相机管理
        template<typename T, typename... Args>
        T* createCamera(const std::string& name, Args&&... args);

        void addCamera(const std::string& name, ICamera* camera);

        void removeCamera(const std::string& name);

        void switchCamera(uint32_t cameraID);
        void switchCamera(const std::string& name);
        ICamera* getActiveCamera() const;
        uint32_t getActiveCameraID() const;
        uint32_t getCameraID(const std::string& name) const;
        ICamera* getCamera(uint32_t cameraID) const;
        ICamera* getCamera(const std::string& name) const;

        // 更改窗口大小后调用该函数，已嵌入回调
        void setViewportAll(const math::Vector4& viewport);

    private:
        std::unordered_map<uint32_t, std::unique_ptr<ICamera>> _cameras;
        std::unordered_map<std::string, uint32_t> _cameraMap;
        ICamera* _activeCamera = nullptr;
        uint32_t _currentIndex = 0; // 0代表当前没有有效相机
        uint32_t _maxIndex = 0;     // 0代表当前没有有效相机
    };

    template<typename T, typename... Args>
    inline T* CameraSystem::createCamera(const std::string& name, Args&&... args)
    {
        auto camera = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = camera.get();
        _cameras[++_maxIndex] = std::move(camera);
        _cameraMap[name] = _maxIndex;
        return ptr;
    }
}