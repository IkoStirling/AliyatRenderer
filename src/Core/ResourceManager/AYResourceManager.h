#pragma once
#include "Core/EventSystem/core_event_system.h"
#include "Core/EngineCore/AYEngineCore.h"
#include "IAYResource.h"
#include "STResourceLoadRequest.h"
#include "Core/SystemResourceType/AYTexture.h"
#include "Core/SystemEventType/Event_ResourceLoadAsync.h"
#include <memory>
#include <unordered_map>
#include <future>

template <typename T>
class AYResourceHandle;

class AYResourceManager {
public:
    static AYResourceManager& getInstance()
    {
        static AYResourceManager mInstance;
        return mInstance;
    }
public:
    /*
        立即加载资源
    */
    template <typename T, typename... Args>
    std::shared_ptr<T> load(const std::string& filepath, Args&& ...args);

    /*
        异步加载资源
    */
    template<typename T>
    std::future<std::shared_ptr<T>> loadAsync(const std::string& path);

    /*
        返回资源句柄
    */
     template <typename T>
     std::shared_ptr<AYResourceHandle<T>> createHandle(const std::string& filepath);

     /*
         每次手动卸载后，都会尝试清理失效缓存？
     */
    void unload(const std::string& filepath) {
        auto it = _resourcesCache.find(filepath);
        if (it != _resourcesCache.end()) {
            if (auto resource = it->second.lock())
            {
                if (resource->unload())
                {
                    _unloadUnusedResources();
                }
            }
        }
    }

private:
    AYResourceManager() {
        _listenEvents<AYTexture>();
    }
    ~AYResourceManager() {}

    void _unloadUnusedResources()
    {
        for (auto it = _resourcesCache.begin(); it != _resourcesCache.end(); )
        {
            if (it->second.expired())
                it = _resourcesCache.erase(it);
            else
                it++;
        }
    }
private:
    std::unordered_map<std::string, std::weak_ptr<IAYResource>> _resourcesCache;

private:
    template<typename T>
    void _listenEvents();

    std::vector<std::unique_ptr<AYEventToken>> _tokens;
};












#include "AYResourceHandle.h"

template<typename T, typename ...Args>
inline std::shared_ptr<T> AYResourceManager::load(const std::string& filepath, Args && ...args)
{
    auto it = _resourcesCache.find(filepath);
    if (it != _resourcesCache.end()) {
        if (auto resource = it->second.lock())
            return std::dynamic_pointer_cast<T>(resource);
    }

    auto resource = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
    if (!resource->load(filepath)) {
        return nullptr;
    }
    _resourcesCache[filepath] = resource;
    return resource;
}

template<typename T>
inline std::future<std::shared_ptr<T>> AYResourceManager::loadAsync(const std::string& path)
{
    auto promise = std::make_shared<std::promise<std::shared_ptr<T>>>();
    auto future = promise->get_future();

    STResourceLoadRequest<T> request{ path, promise };

    // 发布事件
    AYEventRegistry::publish(GetEventSystem(), Event_ResourceLoadAsync<T>::staticGetType(), [&request](IAYEvent* event) {
        auto eI = static_cast<Event_ResourceLoadAsync<T>*>(event);
        eI->mRequest = request;
        });
    return future;
}

template<typename T>
inline std::shared_ptr<AYResourceHandle<T>> AYResourceManager::createHandle(const std::string& filepath)
{
    return std::shared_ptr<AYResourceHandle<T>>(filepath);
}

template<typename T>
inline void AYResourceManager::_listenEvents()
{
    auto loader = [this](const IAYEvent& in_event) {
        auto event = static_cast<const Event_ResourceLoadAsync<T>&>(in_event);
        auto request = event.mRequest;
        auto result = this->load<T>(request.mPath);
        request.mPromise->set_value(result);
        };
    auto token = SUBSCRIBE_EVENT_LAMBDA(GetEventSystem(), Event_ResourceLoadAsync<T>::staticGetType(), loader);
    _tokens.push_back(std::unique_ptr<AYEventToken>(token));
}
