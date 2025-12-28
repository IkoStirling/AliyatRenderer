#include "AYPath.h"
#include <iostream>

namespace ayt::engine::path
{
    // 初始化静态成员变量
    std::once_flag Path::_initFlag;
    const std::string Path::Windows::AppName = "AliyatRenderer";

    const std::string Path::Engine::AssetsPath = "assets/";
    const std::string Path::Engine::PresetPath = Path::Engine::AssetsPath + "core/";
    const std::string Path::Engine::UserPath = Path::Engine::AssetsPath + "usrs/";
    const std::string Path::Engine::PresetTexturePath = Path::Engine::PresetPath + "textures/";
    const std::string Path::Engine::PresetCachePath = Path::Engine::PresetPath + "cache/";
    const std::string Path::Engine::PresetShaderPath = Path::Engine::PresetPath + "shaders/";
    const std::string Path::Engine::PresetConfigPath = Path::Engine::PresetPath + "config/";
    const std::string Path::Engine::PresetAudioPath = Path::Engine::PresetPath + "audios/";
    const std::string Path::Engine::PresetVideoPath = Path::Engine::PresetPath + "videos/";
    const std::string Path::Engine::PresetWebPath = Path::Engine::PresetPath + "web/";
    const std::string Path::Engine::PresetModelPath = Path::Engine::PresetPath + "models/";


    std::string Path::Engine::getAssetsPath() { return AssetsPath; }
    std::string Path::Engine::getPresetPath() { return PresetPath; }
    std::string Path::Engine::getUserPath() { return UserPath; }
    std::string Path::Engine::getPresetModelPath() { return PresetModelPath; }
    std::string Path::Engine::getPresetTexturePath() { return PresetTexturePath; }
    std::string Path::Engine::getPresetCachePath() { return PresetCachePath; }
    std::string Path::Engine::getPresetShaderPath() { return PresetShaderPath; }
    std::string Path::Engine::getPresetConfigPath() { return PresetConfigPath; }
    std::string Path::Engine::getPresetAudioPath() { return PresetAudioPath; }
    std::string Path::Engine::getPresetVideoPath() { return PresetVideoPath; }
    std::string Path::Engine::getPresetWebPath() { return PresetWebPath; }

    std::string Path::Windows::getAppCachePath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\cache\\";
        }
        return Path::Engine::getPresetCachePath(); // 返回默认路径（虽然可能不准确，但避免崩溃）
    }

    std::string Path::Windows::getRomingPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return Path::Engine::getPresetCachePath();
    }

    std::string Path::Windows::getTempPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return Path::Engine::getPresetCachePath();
    }

    std::string Path::Windows::getDesktopPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return ""; // 桌面路径获取失败返回空字符串
    }

    std::string Path::Windows::getDocumentsPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return Path::Engine::getPresetPath() + "documents\\"; // 返回默认路径
    }

    std::string Path::Windows::getPicturesPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_PICTURES, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return Path::Engine::getPresetPath() + "pictures\\"; // 返回默认路径
    }

    std::string Path::Windows::getMusicPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_MUSIC, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return Path::Engine::getPresetPath() + "music\\"; // 返回默认路径
    }

    std::string Path::Windows::getVideosPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_VIDEO, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return Path::Engine::getPresetPath() + "videos\\"; // 返回默认路径
    }

    std::string Path::Windows::getFontsPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, path)))
        {
            return std::string(path) + "\\" + AppName + "\\";
        }
        return Path::Engine::getPresetPath() + "font\\"; // 返回默认路径
    }

    // 静态成员初始化
    std::vector<std::string> Path::Resolver::_searchPaths;
    std::unordered_map<std::string, std::string> Path::Resolver::_aliases;
    std::unordered_map<std::string, std::string> Path::Resolver::_resolvedCache;
    std::mutex Path::Resolver::_mutex;
    std::string Path::Resolver::_assetsRoot = "";

    void Path::Resolver::addSearchPath(const std::string& path) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string normalized = _normalize(path);
        if (std::find(_searchPaths.begin(), _searchPaths.end(), normalized) == _searchPaths.end()) {
            _searchPaths.push_back(normalized);
        }
    }

    void Path::Resolver::setAlias(const std::string& alias, const std::string& path) {
        std::lock_guard<std::mutex> lock(_mutex);
        _aliases[_normalize(alias)] = _normalize(path);
    }

    void Path::Resolver::setAssetsRoot(const std::string& rootPath)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _assetsRoot = _normalize(rootPath);
    }

    std::string Path::Resolver::resolve(const std::string& inputPath)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (auto it = _resolvedCache.find(inputPath); it != _resolvedCache.end()) {
            return it->second; // 返回缓存结果
        }

        std::string path = _normalize(inputPath);
        std::string finalPath = path;

        // 1. 检查是否为绝对路径或已存在
        if (fileExists(path)) {
            return path;
        }

        // 2. 尝试替换别名
        bool isAlias = false; //是否尝试过替换
        for (const auto& [alias, aliasPath] : _aliases) {
            if (path.compare(0, alias.size(), alias) == 0) {    // 已经找到别名
                std::string remaining = path.substr(alias.size());
                std::string fullpath = _combine(aliasPath, remaining);

                if (fileExists(fullpath)) {
                    _resolvedCache[path] = fullpath;
                    return fullpath;
                }
                finalPath = fullpath;
                isAlias = true; // 标记为已经进行别名替换
            }
        }

        // 3. 尝试在搜索路径中查找
        for (const auto& base : _searchPaths) {
            std::string fullpath = _combine(base, path);
            if (fileExists(fullpath)) {
                _resolvedCache[path] = fullpath;
                return fullpath;
            }
        }

        // 4. 尝试在资源根目录下查找
        if (!_assetsRoot.empty()) {
            std::string fullpath = _combine(_assetsRoot, path);
            if (fileExists(fullpath)) {
                _resolvedCache[path] = fullpath;
                return fullpath;
            }
        }

        if (!isAlias && finalPath == path)
            std::cerr << "[Path::Resolver] No replacement alias: " << finalPath << "\n";
        return finalPath;
    }

    bool Path::Resolver::fileExists(const std::string& path) {
        std::ifstream f(path);
        return f.good();
    }

    std::string Path::Resolver::_combine(const std::string& base, const std::string& relative) {
        if (base.empty()) return relative;
        if (relative.empty()) return base;

        std::string result = base;
        if (result.back() != '/' && result.back() != '\\') {
            result += '/';
        }

        // 移除relative开头可能存在的斜杠
        std::string rel = relative;
        while (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) {
            rel.erase(0, 1);
        }

        return result + rel;
    }

    std::string Path::Resolver::_normalize(const std::string& path) {
        std::string result = path;
        // 统一使用正斜杠
        std::replace(result.begin(), result.end(), '\\', '/');
        // 移除多余的斜杠
        auto new_end = std::unique(result.begin(), result.end(),
            [](char l, char r) { return l == '/' && r == '/'; });
        result.erase(new_end, result.end());
        return result;
    }
}