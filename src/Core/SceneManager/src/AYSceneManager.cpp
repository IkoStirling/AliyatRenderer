#include "AYSceneManager.h"

void AYSceneManager::init()
{
	// ��ʼ������������
	_sceneRegistry.clear();
	_activeScenes.clear();
}

void AYSceneManager::update(float delta_time)
{
	// ��������س���
	processPendingScene();

	// �������л����
	for (auto* scene : _activeScenes) {
		if (scene) {
			scene->update(delta_time);
		}
	}

}

void AYSceneManager::shutdown()
{
	// ж�����г���
	for (auto* scene : _activeScenes) {
		if (scene) {
            scene->endPlay();
			scene->unload();
		}
	}
	_activeScenes.clear();
	_sceneRegistry.clear();
}

void AYSceneManager::loadScene(const std::string& name, LoadMode mode)
{
	auto it = _sceneRegistry.find(name);
	if (it == _sceneRegistry.end())
		return;

	// ���ô����س���
	_pendingScene = std::move(it->second);
	_pendingLoadMode = mode;

	// ��ע������Ƴ�������Ȩת�Ƶ�_pendingScene
	_sceneRegistry.erase(it);
}

void AYSceneManager::unloadScene(const std::string& name)
{
    // �ӻ�����б����Ƴ�
    _activeScenes.erase(
        std::remove_if(_activeScenes.begin(), _activeScenes.end(),
            [&name](IAYScene* scene) {
                return scene && scene->getName() == name;
            }),
        _activeScenes.end()
    );

    // ��ע������Ƴ�
    auto it = _sceneRegistry.find(name);
    if (it != _sceneRegistry.end()) {
        it->second->unload();
        _sceneRegistry.erase(it);
    }
}

void AYSceneManager::processPendingScene()
{
    if (!_pendingScene) return;

    // ���ݼ���ģʽ����ǰ����
    switch (_pendingLoadMode) {
    case LoadMode::Single:
        // ж�����е�ǰ����
        for (auto* scene : _activeScenes) {
            if (scene) {
                scene->endPlay();
                scene->unload();
            }
        }
        _activeScenes.clear();
        break;

    case LoadMode::Additive:
        // ������ǰ����
        break;
    }

    // �����³���
    _pendingScene->load();
    _pendingScene->beginPlay();

    // ��ӵ�������б�
    _activeScenes.push_back(_pendingScene.get());

    // ����������Ȩת�ƻ�ע���
    _sceneRegistry[_pendingScene->getName()] = std::move(_pendingScene);
}

bool AYSceneManager::hasScene(const std::string& name) const
{
    return _sceneRegistry.find(name) != _sceneRegistry.end();
}

void AYSceneManager::removeScene(const std::string& name)
{
    // �ȴӻ�������Ƴ�
    unloadScene(name);
    // Ȼ���ע������Ƴ�
    _sceneRegistry.erase(name);
}