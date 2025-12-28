#include "AYResourceManager.h"
#include "AYResourceRegistry.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include "AYTexture.h"
#include "AYAudio.h"
#include "AYVideo.h"
#include "AYLogger.h"
namespace ayt::engine::resource
{
    using json = nlohmann::json;


    ResourceManager& ResourceManager::getInstance()
    {
        static ResourceManager mInstance;
        return mInstance;
    }

    void ResourceManager::unloadResource(const std::string& filepath) {
        auto strongIt = _strongCache.find(filepath);
        if (strongIt != _strongCache.end()) {
            if (strongIt->second.resource->unload())
            {
                _strongCache.erase(strongIt);
                _cleanupResources();
            }
        }
    }

    void ResourceManager::reloadResource(const std::string& filepath) {
        std::string rpath = Path::resolve(filepath);
        auto resource = getResourceByPath(filepath);
        resource->reload(rpath);
    }

    void ResourceManager::pinResource(const std::string& filepath, const std::shared_ptr<IResource>& res)
    {
        size_t size = res->sizeInBytes();
        _currentMemoryUsage += size;
        _strongCache[filepath] = STCacheEntry{ res, size, std::chrono::steady_clock::now() };
    }

    void ResourceManager::unpinResource(const std::string& filepath)
    {
        _strongCache.erase(filepath);
    }

    void ResourceManager::printStats()
    {
        AYLOG_INFO("[ResourceManager] === Resource Cache Stats ===");
        for (const auto& [filepath, weak] : _weakCache) {
            if (auto shared = weak.lock()) {
                // 减去当前 lock() 产生的引用，只统计外部持有的引用
                int count = shared.use_count() - 1;
                AYLOG_INFO("{} - use_count: {}", filepath, count);
            }
            else {
                AYLOG_INFO("{} - use_count: 0", filepath);
            }
        }
    }

    void ResourceManager::touchResource(const std::string& filepath)
    {
        std::string rpath = Path::resolve(filepath);
        auto strongIt = _strongCache.find(rpath);
        if (strongIt != _strongCache.end()) {
            strongIt->second.lastUsed = std::chrono::steady_clock::now();
        }
    }

    void ResourceManager::trim()
    {
        while (_strongCache.size() > _maxItemCount || _currentMemoryUsage > _maxMemoryBytes) {
            auto oldest = _strongCache.begin();
            for (auto it = _strongCache.begin(); it != _strongCache.end(); ++it) {
                if (it->second.lastUsed < oldest->second.lastUsed)
                    oldest = it;
            }

            _currentMemoryUsage -= oldest->second.size;
            _strongCache.erase(oldest);
        }
    }

    void ResourceManager::update(float delta_time)
    {
        AsyncTracker::getInstance().update(delta_time);
    }

    void ResourceManager::init()
    {
        _listenEvents();
    }

    void ResourceManager::shutdown()
    {
        //std::unordered_map<std::string, std::weak_ptr<IResource>> wc;
        //std::unordered_map<std::string, STCacheEntry> sc;
        //_weakCache.swap(wc);
        //_strongCache.swap(sc);
    }

    std::shared_ptr<IResource> ResourceManager::getResourceByPath(const std::string& filepath)
    {
        auto strongIt = _strongCache.find(filepath);
        if (strongIt != _strongCache.end()) {
            return strongIt->second.resource;
        }

        auto weakIt = _weakCache.find(filepath);
        if (weakIt != _weakCache.end()) {
            if (auto resource = weakIt->second.lock())
            {
                pinResource(filepath, resource);
                return resource;
            }
        }

        return nullptr;
    }

    void ResourceManager::tagResource(const std::string& filepath, const Tag& tag)
    {
        std::string rpath = Path::resolve(filepath);
        _tagMap[tag].insert(rpath);
        if (auto res = getResourceByPath(rpath)) {
            res->addTag(tag);
        }
    }

    void ResourceManager::untagResource(const std::string& filepath, const Tag& tag)
    {
        std::string rpath = Path::resolve(filepath);
        _tagMap[tag].erase(rpath);
        if (_tagMap[tag].empty()) {
            _tagMap.erase(tag);
        }

        if (auto res = getResourceByPath(rpath)) {
            res->removeTag(tag);
        }
    }

    std::vector<std::shared_ptr<IResource>> ResourceManager::getResourcesWithTag(const Tag& tag)
    {
        std::vector<std::shared_ptr<IResource>> result;
        auto it = _tagMap.find(tag);
        if (it != _tagMap.end()) {
            for (const auto& path : it->second) {
                if (auto res = getResourceByPath(path)) {
                    result.push_back(res);
                }
            }
        }
        return result;
    }

    void ResourceManager::unloadTag(const Tag& tag)
    {
        auto resources = getResourcesWithTag(tag);
        for (auto& res : resources) {
            unloadResource(res->getPath()); // 假设资源接口中支持 getPath()
        }
    }

    void ResourceManager::printTaggedStats(const Tag& tag)
    {
        auto resources = getResourcesWithTag(tag);
        AYLOG_INFO("[ResourceManager] Resources with tag [{}]:", tag);
        for (auto& res : resources) {
            AYLOG_INFO("{} | size: {}", res->getPath(), res->sizeInBytes());
        }
    }

    void ResourceManager::savePersistentCache(const std::string& savePath)
    {
        // 暂时弃用，未支持序列化参数

        //json j;
        //for (const auto& [path, entry] : _strongCache) {
        //    j[path] = {
        //        {"size", entry.size},
        //        {"lastUsed", std::chrono::duration_cast<std::chrono::seconds>(
        //            entry.lastUsed.time_since_epoch()).count()},
        //        {"type", entry.resource->getType()}
        //    };
        //}
        //std::ofstream out(savePath);
        //out << j.dump(4);
    }

    void ResourceManager::loadPersistentCache(const std::string& loadPath)
    {
        // 暂时弃用，未支持序列化参数

        //std::ifstream in(loadPath);
        //if (!in) return;
        //json j;
        //in >> j;
        //for (auto& [path, val] : j.items()) {
        //    std::string typeName = val.at("type").get<std::string>();
        //    auto resource = ResourceRegistry::getInstance().create<IResource>(typeName);
        //    if (resource && resource->load(path)) {
        //        size_t size = resource->sizeInBytes();
        //        _strongCache[path] = STCacheEntry{
        //            resource,
        //            size,
        //            std::chrono::steady_clock::now()
        //        };
        //        _currentMemoryUsage += size;
        //        _weakCache[path] = resource;
        //    }
        //}
    }

    void ResourceManager::_preloadFromConfig(const std::string& configPath)
    {
        // 暂时弃用，未支持序列化参数

        //std::ifstream in(configPath);
        //if (!in) return;
        //json j;
        //in >> j;
        //if (j.contains(AYTexture::staticGetType()))
        //{
        //    for (const auto& tex : j[AYTexture::staticGetType()])
        //    {
        //        loadAsync<AYTexture>(tex.get<std::string>());
        //    }
        //}

        //for (const auto& tex : j["textures"]) {
        //    loadAsync<AYTexture>(tex); // 或 load<> 同步加载
        //}

        /*for (const auto& mesh : j["meshes"]) {
            loadAsync<AYMesh>(mesh);
        }*/

        // 同理 for sounds、shaders 等等
    }

    ResourceManager::ResourceManager()
    {
    }

    ResourceManager::~ResourceManager()
    {
    }

    void ResourceManager::_cleanupResources()
    {
        for (auto it = _weakCache.begin(); it != _weakCache.end(); )
        {
            if (it->second.expired())
                it = _weakCache.erase(it);
            else
                it++;
        }
    }

    void ResourceManager::_listenEvents()
    {
        registerResourceType<AYTexture>();
        registerResourceType<AYAudio>();
        registerResourceType<AYAudioStream>();
        registerResourceType<AYVideo>();
        registerResourceType<AYModel>();
    }
}