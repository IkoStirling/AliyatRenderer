#pragma once
#include "core_event_system.h"
#include "IAYResource.h"
#include "STResourceLoadRequest.h"

#include "AYEventRegistry.h"
#include "AYAsyncTracker.h"
#include "AYTexture.h"

#include "Mod_ResourceManager.h"
#include "Event_ResourceLoadAsync.h"

#include <memory>
#include <unordered_map>
#include <future>



template <typename T>
class AYResourceHandle;


/*
    后续：
    资源依赖
    资源优先级
    持久化缓存/预加载列表
*/


class AYResourceManager {
public:
    using Tag = std::string;
    using TagSet = std::unordered_set<Tag>;
public:
    static AYResourceManager& getInstance();
public:
    /*
        立即加载资源（异步加载中加载功能，由该函数实现）
        该函数实现功能：
            a）加载资源
            b）资源强弱缓存
            c）缓存数量/大小控制（基于LRU）
            d）如已加载，则从强弱缓存中获取对象
        关于强弱缓存的意义：
            强缓存由资源管理器直接持有，保证资源持久化
            弱缓存不持有资源，只持有引用，如果强缓存因缓存管理被删除，但是外界还持有资源，则从弱缓存获取
            须注意，通过弱缓存实现的加载，不会在资源管理器中缓存资源
    */
    template <typename T, typename... Args>
    std::shared_ptr<T> load(const std::string& filepath, Args&& ...args);


    /*
        异步加载资源
        该函数实现功能：
            a）返回资源共享指针的future，以供手动阻塞获取 （不建议在主线程使用）
            b）发布异步加载事件，由事件系统处理
            c）实际调用立即加载函数，因此异步加载也会进行缓存
            d）事件加载完成时执行回调
            e）异步任务追踪 （目前看起来没什么用？可以添加函数查看异步任务追踪器中的任务以完善）
    */
    template<typename T, typename ...Args>
    std::shared_future<std::shared_ptr<T>> loadAsync(
        const std::string& filepath,
        Args&&... args,
        std::function<void(std::shared_ptr<T>)> callback = {}
        );


    /*
        返回资源句柄
        该函数实现功能：
            a）通过资源句柄创建的资源，将脱离缓存，需要手动管理，但可以实现延迟加载
    */
     template <typename T>
     std::shared_ptr<AYResourceHandle<T>> createHandle(const std::string& filepath);


     /*
         手动卸载资源
         默认不解析路径
         该函数实现功能：
            a）将资源卸载，并从强弱缓存中移除
     */
     void unloadResource(const std::string& filepath);


    /*
        重载资源
        该函数实现功能：
            a）热重载
    */
     void reloadResource(const std::string& filepath);


    /*
        钉住资源
        默认不解析路径
        该函数实现功能：
            a）将资源加入强缓存
    */
     void pinResource(const std::string& filepath, const std::shared_ptr<IAYResource>& res);


    /*
        取消持有资源
        默认不解析路径
        该函数实现功能：
            a）将资源移除强缓存
    */
     void unpinResource(const std::string& filepath);

    /*
        DEBUG
    */
     void printStats();


    /*
        更新访问时间
        该函数实现功能：
            a）设置强缓存的time point
    */
     void touchResource(const std::string& filepath);


    /*
        淘汰缓存
        该函数实现功能：
            a）LRU淘汰缓存
    */
     void trim();


    /*
        TICK
    */
     void update(float delta_time);

     /*
        初始化
     */
     void init();

     /*
     
     */
     void shutdown();

     /*
        手动注册要纳入管理的资源类型（继承自IAYResource，与资源类型的自动注册是两码事）
        该函数实现功能：（在异步加载事件完成时执行以下操作）
            a) 完成异步承诺，异步加载得到的future会得到结果
            b）执行回调通知，如果异步加载设置有回调会执行
            c）异常处理，由承诺传递
     */
    template<typename T, typename ...Args>
    void registerResourceType();

    /*
        底层接口，不建议调用
        默认不解析路径
     */
    std::shared_ptr<IAYResource> getResourceByPath(const std::string& filepath);

    void tagResource(const std::string& filepath, const Tag& tag);

    void untagResource(const std::string& filepath, const Tag& tag);

    std::vector<std::shared_ptr<IAYResource>> getResourcesWithTag(const Tag& tag);

    void unloadTag(const Tag& tag);

    void printTaggedStats(const Tag& tag);

    void savePersistentCache(const std::string& savePath);
    void loadPersistentCache(const std::string& loadPath);

    ~AYResourceManager();
private:
    AYResourceManager();

    /*
        清理失效的weakCache
    */
    void _cleanupResources();
private:
    struct STCacheEntry {
        std::shared_ptr<IAYResource> resource;
        size_t size;
        std::chrono::steady_clock::time_point lastUsed;
    };

    struct PersistentCacheEntry {
        std::string filepath;
        size_t size;
        std::time_t lastUsed;
        std::string typeName; 
    };

    std::unordered_map<std::string, std::weak_ptr<IAYResource>> _weakCache;
    std::unordered_map<std::string, STCacheEntry> _strongCache;

    size_t _maxItemCount = 200;           // 最大缓存个数
    size_t _maxMemoryBytes = 512 * 1024 * 1024; // 最大缓存大小，单位字节
    size_t _currentMemoryUsage = 0;

private:
    void _listenEvents();
    std::vector<std::unique_ptr<AYEventToken>> _tokens;

private:
    std::unordered_map<Tag, std::unordered_set<std::string>> _tagMap;

private:
    void _preloadFromConfig(const std::string& configPath);
};












#include "AYResourceHandle.h"

template<typename T, typename ...Args>
inline std::shared_ptr<T> AYResourceManager::load(const std::string& filepath, Args && ...args)
{
    std::string rpath = AYPath::resolve(filepath);
    auto strongIt = _strongCache.find(rpath);
    if (strongIt != _strongCache.end())
    {
        touchResource(rpath);
        return std::static_pointer_cast<T>(strongIt->second.resource);
    }

    //类型确认，可以直接使用static cast
    auto weakIt = _weakCache.find(rpath);
    if (weakIt != _weakCache.end()) {
        if (auto resource = weakIt->second.lock())
        {
            pinResource(rpath, resource);    //刷新强缓存
            touchResource(rpath);
            return std::dynamic_pointer_cast<T>(resource);
        }
    }

    try {
        // 使用资源注册表创建资源实例
        auto resource = AYResourceRegistry::getInstance().create<T>(T::staticGetType(), std::forward<Args>(args)...);
        if (!resource) {
            // 如果注册表中没有找到，回退到直接创建
            resource = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
        }

        if (!resource->load(rpath))
            throw std::runtime_error("Failed to load resource: " + rpath);
        
        _weakCache[rpath] = resource;
        pinResource(rpath, resource);
        trim();

        return resource;
    }
    catch (const std::exception& e)
    {
        spdlog::error("[AYResourceManager] Error loading resource: {}", e.what());
        return nullptr;
    }
}


template<typename T, typename ...Args>
inline std::shared_future<std::shared_ptr<T>> AYResourceManager::loadAsync(
    const std::string& filepath,
    Args&&... args,
    std::function<void(std::shared_ptr<T>)> callback
)
{
    //std::string rpath = AYPath::resolve(filepath);
    auto promise = std::make_shared<std::promise<std::shared_ptr<T>>>();
    auto future = promise->get_future().share();

    auto castPromise = std::make_shared<std::promise<std::shared_ptr<IAYResource>>>();
    auto castFuture = castPromise->get_future().share();

    auto castCallback = [castPromise, callback](std::shared_ptr<T> result) {
        castPromise->set_value(std::static_pointer_cast<IAYResource>(result));
        if (callback) callback(result);
        };

    auto request = std::make_shared<STResourceLoadRequest<T, Args...>>(
        filepath,
        promise,
        castCallback,
        std::make_tuple(std::forward<Args>(args)...)
    );

    AYAsyncTracker::getInstance().addTask(
        filepath,
        castFuture,
        nullptr,
        std::chrono::seconds(10)
    );
    // 发布事件
    AYEventRegistry::publish(Event_ResourceLoadAsync<T, Args...>::staticGetType(),
        [request = std::move(request)](IAYEvent* event)  mutable {
        auto eI = static_cast<Event_ResourceLoadAsync<T, Args...>*>(event);
        eI->mRequest = std::move(request);
        });
    return future;
}



template<typename T>
inline std::shared_ptr<AYResourceHandle<T>> AYResourceManager::createHandle(const std::string& filepath)
{
    std::string rpath = AYPath::resolve(filepath);
    return std::shared_ptr<AYResourceHandle<T>>(rpath);
}



template<typename T, typename ...Args>
inline void AYResourceManager::registerResourceType()
{
    auto loader = [this](const IAYEvent& in_event) 
    {
        auto& event = static_cast<const Event_ResourceLoadAsync<T>&>(in_event);
        const auto& request = event.mRequest;

        try {
            auto result = std::apply([this, &request](auto&&... args) {
                return this->load<T>(request->mPath, std::forward<decltype(args)>(args)...);
                }, request->mArgs);

            if(!result)
                throw std::runtime_error("Async load failed: " + request->mPath);

            request->mPromise->set_value(result);

            if (request->mCallback)
                request->mCallback(result);
        }
        catch (...)
        {
            try {
                request->mPromise->set_exception(std::current_exception());
            }
            catch (...) 
            { }
        }
    };

    auto system = GET_CAST_MODULE(AYEventSystem, "EventSystem");
    if (system)
    {
        auto token = system->subscribe(Event_ResourceLoadAsync<T>::staticGetType(), loader);
        _tokens.push_back(std::unique_ptr<AYEventToken>(token));
    }
}


class AYResourceManagerAdapter : public Mod_ResourceManager
{
public:
    void init() override { AYResourceManager::getInstance().init(); }
    void shutdown() override { AYResourceManager::getInstance().shutdown(); }
    void update(float delta_time) override { AYResourceManager::getInstance().update(delta_time); }

public:
    static AYResourceManager& getInstance() { return AYResourceManager::getInstance(); }
};

REGISTER_MODULE_CLASS("ResourceManager", AYResourceManagerAdapter)