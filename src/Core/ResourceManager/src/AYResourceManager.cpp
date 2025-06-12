#include "AYResourceManager.h"
#include "AYResourceRegistry.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;


AYResourceManager& AYResourceManager::getInstance()
{
    static AYResourceManager mInstance;
    return mInstance;
}

void AYResourceManager::unloadResource(const std::string& filepath) {
    auto strongIt = _strongCache.find(filepath);
    if (strongIt != _strongCache.end()) {
        if (strongIt->second.resource->unload())
        {
            _strongCache.erase(strongIt);
            _cleanupResources();
        }
    }
}

void AYResourceManager::reloadResource(const std::string& filepath) {
    auto strongIt = _strongCache.find(filepath);
    if (strongIt != _strongCache.end()) {
        strongIt->second.resource->reload(filepath);
        return;
    }

    auto weakIt = _weakCache.find(filepath);
    if (weakIt != _weakCache.end()) {
        if (auto resource = weakIt->second.lock())
        {
            resource->reload(filepath);
        }
    }
}

void AYResourceManager::pinResource(const std::string& filepath, const std::shared_ptr<IAYResource>& res)
{
    size_t size = res->sizeInBytes();
    _currentMemoryUsage += size;
    _strongCache[filepath] = STCacheEntry{ res, size, std::chrono::steady_clock::now() };
}

void AYResourceManager::unpinResource(const std::string& filepath)
{
    _strongCache.erase(filepath);
}

void AYResourceManager::printStats()
{
    std::cout << "=== Resource Cache Stats ===\n";
    for (const auto& [filepath, weak] : _weakCache) {
        auto shared = weak.lock();
        //此处作用域会占用一个引用计数
        int count = shared ? shared.use_count() - 1 : 0;
        std::cout << filepath << " - use_count: " << count << "\n";
    }
}

void AYResourceManager::touchResource(const std::string& filepath)
{
    auto strongIt = _strongCache.find(filepath);
    if (strongIt != _strongCache.end()) {
        strongIt->second.lastUsed = std::chrono::steady_clock::now();
    }
}

void AYResourceManager::trim()
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

void AYResourceManager::update(float delta_time)
{
    AYAsyncTracker::getInstance().update(delta_time);
}

void AYResourceManager::init()
{
    _listenEvents();
    loadPersistentCache("assets/core/config/persistentResources.json");
    std::cout << "AYResourceManager::init()\n";
    //_preloadFromConfig("assets/core/preloadResource.json");
}

std::shared_ptr<IAYResource> AYResourceManager::getResourceByPath(const std::string& filepath)
{
    auto strongIt = _strongCache.find(filepath);
    if (strongIt != _strongCache.end()) {
        return strongIt->second.resource;
    }

    auto weakIt = _weakCache.find(filepath);
    if (weakIt != _weakCache.end()) {
        if (auto resource = weakIt->second.lock())
        {
            return resource;
        }
    }

    return nullptr;
}

void AYResourceManager::tagResource(const std::string& filepath, const Tag& tag)
{
    _tagMap[tag].insert(filepath);
    if (auto res = getResourceByPath(filepath)) {
        res->addTag(tag);
    }
}

void AYResourceManager::untagResource(const std::string& filepath, const Tag& tag)
{
    _tagMap[tag].erase(filepath);
    if (_tagMap[tag].empty()) {
        _tagMap.erase(tag);
    }

    if (auto res = getResourceByPath(filepath)) {
        res->removeTag(tag);
    }
}

std::vector<std::shared_ptr<IAYResource>> AYResourceManager::getResourcesWithTag(const Tag& tag)
{
    std::vector<std::shared_ptr<IAYResource>> result;
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

void AYResourceManager::unloadTag(const Tag& tag)
{
    auto resources = getResourcesWithTag(tag);
    for (auto& res : resources) {
        unloadResource(res->getPath()); // 假设资源接口中支持 getPath()
    }
}

void AYResourceManager::printTaggedStats(const Tag& tag)
{
    auto resources = getResourcesWithTag(tag);
    std::cout << "Resources with tag [" << tag << "]:\n";
    for (auto& res : resources) {
        std::cout << res->getPath() << " | size: " << res->sizeInBytes() << "\n";
    }
}

void AYResourceManager::savePersistentCache(const std::string& savePath)
{
    json j;

    for (const auto& [path, entry] : _strongCache) {
        j[path] = {
            {"size", entry.size},
            {"lastUsed", std::chrono::duration_cast<std::chrono::seconds>(
                entry.lastUsed.time_since_epoch()).count()},
            {"type", entry.resource->getType()}
        };
    }

    std::ofstream out(savePath);
    out << j.dump(4);
}

void AYResourceManager::loadPersistentCache(const std::string& loadPath)
{
    std::ifstream in(loadPath);
    if (!in) return;

    json j;
    in >> j;


    for (auto& [path, val] : j.items()) {
        std::string typeName = val.at("type").get<std::string>();
        auto resource = AYResourceRegistry::getInstance().create(typeName);
        if (resource && resource->load(path)) {
            size_t size = resource->sizeInBytes();
            _strongCache[path] = STCacheEntry{
                resource,
                size,
                std::chrono::steady_clock::now()
            };
            _currentMemoryUsage += size;
            _weakCache[path] = resource;
        }
    }
}

void AYResourceManager::_preloadFromConfig(const std::string& configPath)
{
    std::ifstream in(configPath);
    if (!in) return;

    json j;
    in >> j;

    if (j.contains(AYTexture::staticGetType()))
    {
        for (const auto& tex : j[AYTexture::staticGetType()])
        {
            loadAsync<AYTexture>(tex.get<std::string>());
        }
    }

    //for (const auto& tex : j["textures"]) {
    //    loadAsync<AYTexture>(tex); // 或 load<> 同步加载
    //}

    /*for (const auto& mesh : j["meshes"]) {
        loadAsync<AYMesh>(mesh);
    }*/

    // 同理 for sounds、shaders 等等
}

AYResourceManager::AYResourceManager()
{

}

AYResourceManager::~AYResourceManager()
{
    //savePersistentCache("assets/core/config/persistentResources.json");
}

void AYResourceManager::_cleanupResources()
{
    for (auto it = _weakCache.begin(); it != _weakCache.end(); )
    {
        if (it->second.expired())
            it = _weakCache.erase(it);
        else
            it++;
    }
}

void AYResourceManager::_listenEvents()
{
    registerResourceType<AYTexture>();
}
