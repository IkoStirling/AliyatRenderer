#pragma once
#include "Mod_SceneManager.h"
#include "IAYScene.h"

class AYSceneManager : public Mod_SceneManager
{
public:
    enum class LoadMode {
        Single,      // ������ģʽ(ж�ص�ǰ)
        Additive     // ����ģʽ(������ǰ)
    };
    AYSceneManager() = default;
    void loadScene(const std::string& name, LoadMode mode = LoadMode::Single);
    void unloadScene(const std::string& name);

    void update(float delta_time);
    void render();

    template<typename T>
    T* addScene(const std::string& name);   //Scene�����޲Σ����ṩ���֧��

private:
    std::unordered_map<std::string, std::unique_ptr<IAYScene>> _sceneRegistry;
    std::vector<IAYScene*> _activeScenes; // ����Ⱦ˳������
    std::unique_ptr<IAYScene> _pendingScene; // �����س���
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
