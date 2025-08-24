#include "AYPath.h"
#include <iostream>

// 初始化静态成员变量
const std::string AYPath::Windows::AppName = "AliyatRenderer";

const std::string AYPath::Engine::AssetsPath = "assets/";
const std::string AYPath::Engine::PresetPath = AYPath::Engine::AssetsPath + "core/";
const std::string AYPath::Engine::UserPath = AYPath::Engine::AssetsPath + "usrs/";
const std::string AYPath::Engine::PresetTexturePath = AYPath::Engine::PresetPath + "textures/";
const std::string AYPath::Engine::PresetCachePath = AYPath::Engine::PresetPath + "cache/";
const std::string AYPath::Engine::PresetShaderPath = AYPath::Engine::PresetPath + "shaders/";
const std::string AYPath::Engine::PresetConfigPath = AYPath::Engine::PresetPath + "config/";
const std::string AYPath::Engine::PresetAudioPath = AYPath::Engine::PresetPath + "audios/";
const std::string AYPath::Engine::PresetVideoPath = AYPath::Engine::PresetPath + "videos/";
const std::string AYPath::Engine::PresetWebPath = AYPath::Engine::PresetPath + "web/";
const std::string AYPath::Engine::PresetModelPath = AYPath::Engine::PresetPath + "models/";


std::string AYPath::Engine::getAssetsPath() { return AssetsPath; }
std::string AYPath::Engine::getPresetPath() { return PresetPath; }
std::string AYPath::Engine::getUserPath() { return UserPath; }
std::string AYPath::Engine::getPresetModelPath() { return PresetModelPath; }
std::string AYPath::Engine::getPresetTexturePath() { return PresetTexturePath; }
std::string AYPath::Engine::getPresetCachePath() { return PresetCachePath; }
std::string AYPath::Engine::getPresetShaderPath() { return PresetShaderPath; }
std::string AYPath::Engine::getPresetConfigPath() { return PresetConfigPath; }
std::string AYPath::Engine::getPresetAudioPath() { return PresetAudioPath; }
std::string AYPath::Engine::getPresetVideoPath() { return PresetVideoPath; }
std::string AYPath::Engine::getPresetWebPath() { return PresetWebPath; }

std::string AYPath::Windows::getAppCachePath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\cache\\";
    }
    return AYPath::Engine::getPresetCachePath(); // 返回默认路径（虽然可能不准确，但避免崩溃）
}

std::string AYPath::Windows::getRomingPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return AYPath::Engine::getPresetCachePath();
}

std::string AYPath::Windows::getTempPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return AYPath::Engine::getPresetCachePath();
}

std::string AYPath::Windows::getDesktopPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return ""; // 桌面路径获取失败返回空字符串
}

std::string AYPath::Windows::getDocumentsPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return AYPath::Engine::getPresetPath() + "documents\\"; // 返回默认路径
}

std::string AYPath::Windows::getPicturesPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_PICTURES, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return AYPath::Engine::getPresetPath() + "pictures\\"; // 返回默认路径
}

std::string AYPath::Windows::getMusicPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_MUSIC, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return AYPath::Engine::getPresetPath() + "music\\"; // 返回默认路径
}

std::string AYPath::Windows::getVideosPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_VIDEO, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return AYPath::Engine::getPresetPath() + "videos\\"; // 返回默认路径
}

std::string AYPath::Windows::getFontsPath()
{
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, path)))
    {
        return std::string(path) + "\\" + AppName + "\\";
    }
    return AYPath::Engine::getPresetPath() + "font\\"; // 返回默认路径
}

// 静态成员初始化
std::vector<std::string> AYPath::Resolver::_searchPaths;
std::unordered_map<std::string, std::string> AYPath::Resolver::_aliases;
std::unordered_map<std::string, std::string> AYPath::Resolver::_resolvedCache;
std::mutex AYPath::Resolver::_mutex;
std::string AYPath::Resolver::_assetsRoot = "";

void AYPath::Resolver::addSearchPath(const std::string& path) {
    std::string normalized = _normalize(path);
    if (std::find(_searchPaths.begin(), _searchPaths.end(), normalized) == _searchPaths.end()) {
        _searchPaths.push_back(normalized);
    }
}

void AYPath::Resolver::setAlias(const std::string& alias, const std::string& path) {
    _aliases[_normalize(alias)] = _normalize(path);
}

void AYPath::Resolver::setAssetsRoot(const std::string& rootPath) {
    _assetsRoot = _normalize(rootPath);
}

std::string AYPath::Resolver::resolve(const std::string& inputPath) {
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
        std::cerr << "[AYPath::Resolver] No replacement alias: " << finalPath << "\n";
    return finalPath;
}

bool AYPath::Resolver::fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

std::string AYPath::Resolver::_combine(const std::string& base, const std::string& relative) {
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

std::string AYPath::Resolver::_normalize(const std::string& path) {
    std::string result = path;
    // 统一使用正斜杠
    std::replace(result.begin(), result.end(), '\\', '/');
    // 移除多余的斜杠
    auto new_end = std::unique(result.begin(), result.end(),
        [](char l, char r) { return l == '/' && r == '/'; });
    result.erase(new_end, result.end());
    return result;
}