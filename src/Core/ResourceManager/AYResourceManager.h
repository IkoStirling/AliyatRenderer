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


/*
    后续：
    资源依赖
    资源标签
    资源优先级
    资源类型注册接口 registerResourceType<AYTexture>()
    异步加载失败
    持久化缓存/预加载列表
*/







struct AsyncTask {
    std::shared_future<std::shared_ptr<IAYResource>> future;
    std::string resourcePath;
};

struct CacheEntry {
    std::shared_ptr<IAYResource> resource;
    size_t size;
    std::chrono::steady_clock::time_point lastUsed;
};

class AYResourceManager {
public:
    static AYResourceManager& getInstance()
    {
        static AYResourceManager mInstance;
        return mInstance;
    }
public:
    size_t _maxItemCount = 200;           // 最大缓存个数
    size_t _maxMemoryBytes = 512 * 1024 * 1024; // 最大缓存大小，单位字节
    size_t _currentMemoryUsage = 0;

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
    std::shared_future<std::shared_ptr<T>> loadAsync(
        const std::string& filepath,
        std::function<void(std::shared_ptr<T>)> callback = nullptr);

    /*
        返回资源句柄
    */
     template <typename T>
     std::shared_ptr<AYResourceHandle<T>> createHandle(const std::string& filepath);

     /*
         每次手动卸载后，都会尝试清理失效缓存？
     */
    void unload(const std::string& filepath) {
        auto it = _weakCache.find(filepath);
        if (it != _weakCache.end()) {
            if (auto resource = it->second.lock())
            {
                if (resource->unload())
                {
                    _cleanupResources();
                }
            }
        }
    }

    void reload(const std::string& filepath) {
        auto strongIt = _strongCache.find(filepath);
        if (strongIt != _strongCache.end()) {
            strongIt->second.resource->reload(filepath);
            return;
        }

        auto weakIt = _weakCache.find(filepath);
        if (weakIt != _weakCache.end()) {
            if (auto resource = weakIt->second.lock())
            {
                weakIt->second.lock()->reload(filepath);
            }
        }
    }
    void pinResource(const std::string& filepath, const std::shared_ptr<IAYResource>& res){
        size_t size = res->sizeInBytes();
        _currentMemoryUsage += size;
        _strongCache[filepath] = CacheEntry{ res, size, std::chrono::steady_clock::now() };
    }
    void unpinResource(const std::string& filepath)
    {
        _strongCache.erase(filepath);
    }
    void printStats() {
        std::cout << "=== Resource Cache Stats ===\n";
        for (const auto& [filepath, weak] : _weakCache) {
            auto shared = weak.lock();
            int count = shared ? shared.use_count() : 0;
            std::cout << filepath << " - use_count: " << count << "\n";
        }
    }

    void touch(const std::string& filepath) {
        auto strongIt = _strongCache.find(filepath);
        if (strongIt != _strongCache.end()) {
            strongIt->second.lastUsed = std::chrono::steady_clock::now();
        }
    } // 更新访问时间

    void trim() {
        while (_strongCache.size() > _maxItemCount || _currentMemoryUsage > _maxMemoryBytes) {
            // 找出最久未使用的资源
            auto oldest = _strongCache.begin();
            for (auto it = _strongCache.begin(); it != _strongCache.end(); ++it) {
                if (it->second.lastUsed < oldest->second.lastUsed)
                    oldest = it;
            }

            // 释放该资源
            _currentMemoryUsage -= oldest->second.size;
            _strongCache.erase(oldest);
        }
    }                       // 淘汰缓存

    void update()
    {
        for (auto it = _inFlightResources.begin(); it != _inFlightResources.end(); ) {
            if (it->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                auto result = it->future.get(); // 多次 get 安全
                pinResource(it->resourcePath, result);
                trim();
                it = _inFlightResources.erase(it);
            }
            else {
                ++it;
            }
        }

    }

private:
    AYResourceManager() {
        _listenEvents<AYTexture>();
    }
    ~AYResourceManager() {}

    void _cleanupResources()
    {
        for (auto it = _weakCache.begin(); it != _weakCache.end(); )
        {
            if (it->second.expired())
                it = _weakCache.erase(it);
            else
                it++;
        }
    }
private:
    std::unordered_map<std::string, std::weak_ptr<IAYResource>> _weakCache;
    std::unordered_map<std::string, CacheEntry> _strongCache;
    std::vector<AsyncTask> _inFlightResources;

private:
    template<typename T>
    void _listenEvents();

    std::vector<std::unique_ptr<AYEventToken>> _tokens;
};












#include "AYResourceHandle.h"

template<typename T, typename ...Args>
inline std::shared_ptr<T> AYResourceManager::load(const std::string& filepath, Args && ...args)
{
    auto strongIt = _strongCache.find(filepath);
    if (strongIt != _strongCache.end())
    {
        touch(filepath);
        return std::static_pointer_cast<T>(strongIt->second.resource);
    }

    //类型确认，可以直接使用static cast
    auto weakIt = _weakCache.find(filepath);
    if (weakIt != _weakCache.end()) {
        if (auto resource = weakIt->second.lock())
        {
            touch(filepath);
            return std::dynamic_pointer_cast<T>(resource);
        }
    }

    auto resource = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
    if (!resource->load(filepath)) {
        return nullptr;
    }

    _weakCache[filepath] = resource;

    pinResource(filepath, resource); 
    trim();

    return resource;
}

template<typename T>
inline std::shared_future<std::shared_ptr<T>> AYResourceManager::loadAsync(
    const std::string& filepath,
    std::function<void(std::shared_ptr<T>)> callback
)
{
    auto promise = std::make_shared<std::promise<std::shared_ptr<T>>>();
    auto future = promise->get_future().share();

    auto castPromise = std::make_shared<std::promise<std::shared_ptr<IAYResource>>>();
    auto castFuture = castPromise->get_future().share();

    auto castCallback = [promise, castPromise, callback](std::shared_ptr<T> result) {
        castPromise->set_value(std::static_pointer_cast<IAYResource>(result));
        if (callback) callback(result);
        };

    STResourceLoadRequest<T> request{ filepath, promise, castCallback };
    _inFlightResources.push_back({ castFuture, filepath });
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

        try {
            auto result = this->load<T>(request.mPath);
            request.mPromise->set_value(result);

            if (request.mCallback)
                request.mCallback(result);
        }
        catch (...)
        {
            request.mPromise->set_exception(std::current_exception());
        }
    };
    auto token = SUBSCRIBE_EVENT_LAMBDA(GetEventSystem(), Event_ResourceLoadAsync<T>::staticGetType(), loader);
    _tokens.push_back(std::unique_ptr<AYEventToken>(token));
}
