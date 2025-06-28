#pragma once
#include "AYGameObject.h"
#include "BaseRendering/IAYRenderable.h"
#include <future>

class IAYScene {
public:
    virtual ~IAYScene() = default;

    //=== ������������ ===//
    virtual void load() = 0;       // ͬ������
    virtual void asyncLoad() = 0;  // �첽�������
    virtual void unload() = 0;
    virtual void beginPlay()
    {
        for (auto& [type, obj] : _objs)
        {
            obj->beginPlay();
        }
    }
    virtual void update(float delta_time)
    {
        for (auto& [type,obj] : _objs)
        {
            obj->update(delta_time);
        }
    }
    virtual void endPlay()
    {
        for (auto& [type, obj] : _objs)
        {
            obj->endPlay();
        }
    }

    //=== �첽���ؽӿ� ===// 
    virtual bool isLoaded() { return true; }
    virtual float getLoadProgress() const { return 1.f; }

    // �������
    template<typename T, typename... Args>
    T* addObject(Args&&... args);

    virtual bool removeObject(const std::string& name) {
        std::lock_guard<std::mutex> lock(_objMutex);
        for (auto it = _objs.begin(); it != _objs.end(); ++it) {
            if (it->second->getName() == name) {
                it->second->endPlay(); // ���������������ڽ���
                _objs.erase(it);
                return true;
            }
        }
        return false;
    }

    // ���ҹ���
    virtual AYGameObject* findObjectByName(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(_objMutex);
        for (auto& [type,obj] : _objs)
        {
            if (obj->getName() == name)
                return obj.get();
        }
        return nullptr;
    }

    template<typename T>
    std::vector<T*> findObjectByType();

    // ������Ϣ
    virtual const std::string& getName() const { return _name; }
    void setName(const std::string& name) { _name = name; }
protected:
    enum class LoadStatus {
        NotLoaded,
        Loading,
        Finished,
        Failed
    };

    // �첽���ؿ���
    std::atomic<LoadStatus> _loadStatus{ LoadStatus::NotLoaded };
    std::atomic<float> _loadProgress{ 0.0f };
    std::future<void> _loadFuture;

    std::unordered_multimap<std::type_index, std::unique_ptr<AYGameObject>> _objs;
    mutable std::mutex _objMutex;
    std::string _name;
};

template<typename T, typename ...Args>
inline T* IAYScene::addObject(Args && ...args)
{
    static_assert(std::is_base_of_v<AYGameObject, T>, "T must inherit from AYGameObject");

    std::lock_guard<std::mutex> lock(_objMutex);
    auto obj = std::make_unique<T>(std::forward<Args>(args)...);
    T* rawPtr = obj.get();
    _objs.emplace(typeid(T), std::move(obj));
    return rawPtr;
}

template<typename T>
inline std::vector<T*> IAYScene::findObjectByType()
{
    static_assert(std::is_base_of_v<AYGameObject, T>, "T must inherit from AYGameObject");

    std::lock_guard<std::mutex> lock(_objMutex);
    std::vector<T*> result;
    auto range = _objs.equal_range(std::type_index(typeid(T)));

    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(static_cast<T*>(it->second.get()));
    }

    return result;
}