#pragma once
#include "Mod_SceneManager.h"
#include "IAYScene.h"
#include <unordered_map>
#include <vector>
#include <memory>
namespace ayt::engine::game
{
    class AYSceneManager : public Mod_SceneManager
    {
    public:
        enum class LoadMode {
            Single,      // 单场景模式(卸载当前)
            Additive     // 叠加模式(保留当前)
        };
        AYSceneManager() = default;
        void loadScene(const std::string& name, LoadMode mode = LoadMode::Single);
        void unloadScene(const std::string& name);

        void init() override;
        void update(float delta_time) override;
        void shutdown() override;

        template<typename T, typename... Args>
        T* addScene(const std::string& name, Args&&... args);

        bool hasScene(const std::string& name) const;
        void removeScene(const std::string& name);

        const std::vector<IAYScene*>& getActiveScenes() const { return _activeScenes; }
        void processPendingScene();
        LoadMode _pendingLoadMode = LoadMode::Single;

    private:
        std::unordered_map<std::string, std::unique_ptr<IAYScene>> _sceneRegistry;
        std::vector<IAYScene*> _activeScenes; // 按渲染顺序排序
        std::unique_ptr<IAYScene> _pendingScene; // 待加载场景
    };

    REGISTER_MODULE_CLASS("SceneManager", AYSceneManager)

        template<typename T, typename... Args>
    inline T* AYSceneManager::addScene(const std::string& name, Args&&... args)
    {
        static_assert(std::is_base_of_v<IAYScene, T>, "T must inherit from IScene");
        if (hasScene(name)) {
            return nullptr; // 场景已存在
        }

        auto scene = std::make_unique<T>(std::forward<Args>(args)...);
        scene->setName(name);

        T* rawPtr = scene.get();
        _sceneRegistry.emplace(name, std::move(scene));
        return rawPtr;
    }
}