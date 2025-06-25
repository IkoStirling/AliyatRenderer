#pragma once
#include "Mod_SceneManager.h"
#include "IAYScene.h"

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

    void update(float delta_time);
    void render();

    template<typename T>
    T* addScene(const std::string& name);   //Scene构造无参，不提供变参支持

private:
    std::unordered_map<std::string, std::unique_ptr<IAYScene>> _sceneRegistry;
    std::vector<IAYScene*> _activeScenes; // 按渲染顺序排序
    std::unique_ptr<IAYScene> _pendingScene; // 待加载场景
};

REGISTER_MODULE_CLASS("SceneManager", AYSceneManager)

template<typename T>
inline T* AYSceneManager::addScene(const std::string& name)
{
    static_assert(std::is_base_of_v<IAYScene, T>, "T must inherit from IScene");
    auto scene = std::make_unique<IAYScene>();
    T* rawPtr = scene.get();

    _sceneRegistry[name] = std::move(scene);
    return rawPtr;
}
