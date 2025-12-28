#include "AYSceneManager.h"
namespace ayt::engine::game
{
    void SceneManager::init()
    {
        // 初始化场景管理器
        _sceneRegistry.clear();
        _activeScenes.clear();
    }

    void SceneManager::update(float delta_time)
    {
        // 处理待加载场景
        processPendingScene();

        // 更新所有活动场景
        for (auto* scene : _activeScenes) {
            if (scene) {
                scene->update(delta_time);
            }
        }

    }

    void SceneManager::shutdown()
    {
        // 卸载所有场景
        for (auto* scene : _activeScenes) {
            if (scene) {
                scene->endPlay();
                scene->unload();
            }
        }
        _activeScenes.clear();
        _sceneRegistry.clear();
    }

    void SceneManager::loadScene(const std::string& name, LoadMode mode)
    {
        auto it = _sceneRegistry.find(name);
        if (it == _sceneRegistry.end())
            return;

        // 设置待加载场景
        _pendingScene = std::move(it->second);
        _pendingLoadMode = mode;

        // 从注册表中移除，所有权转移到_pendingScene
        _sceneRegistry.erase(it);
    }

    void SceneManager::unloadScene(const std::string& name)
    {
        // 从活动场景列表中移除
        _activeScenes.erase(
            std::remove_if(_activeScenes.begin(), _activeScenes.end(),
                [&name](IScene* scene) {
                    return scene && scene->getName() == name;
                }),
            _activeScenes.end()
        );

        // 从注册表中移除
        auto it = _sceneRegistry.find(name);
        if (it != _sceneRegistry.end()) {
            it->second->unload();
            _sceneRegistry.erase(it);
        }
    }

    void SceneManager::processPendingScene()
    {
        if (!_pendingScene) return;

        // 根据加载模式处理当前场景
        switch (_pendingLoadMode) {
        case LoadMode::Single:
            // 卸载所有当前场景
            for (auto* scene : _activeScenes) {
                if (scene) {
                    scene->endPlay();
                    scene->unload();
                }
            }
            _activeScenes.clear();
            break;

        case LoadMode::Additive:
            // 保留当前场景
            break;
        }

        // 加载新场景
        _pendingScene->load();
        _pendingScene->beginPlay();

        // 添加到活动场景列表
        _activeScenes.push_back(_pendingScene.get());

        // 将场景所有权转移回注册表
        _sceneRegistry[_pendingScene->getName()] = std::move(_pendingScene);
    }

    bool SceneManager::hasScene(const std::string& name) const
    {
        return _sceneRegistry.find(name) != _sceneRegistry.end();
    }

    void SceneManager::removeScene(const std::string& name)
    {
        // 先从活动场景中移除
        unloadScene(name);
        // 然后从注册表中移除
        _sceneRegistry.erase(name);
    }
}