#pragma once
#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm> 
#include <mutex>

class AYPath
{
    static std::once_flag _initFlag;
public:
    static void init() {
        std::call_once(_initFlag, []() {
            // 设置资源根目录
            AYPath::Resolver::setAssetsRoot(AYPath::Engine::getAssetsPath());

            // 添加常用搜索路径
            AYPath::Resolver::addSearchPath(AYPath::Engine::getUserPath());
            AYPath::Resolver::addSearchPath(AYPath::Engine::getPresetCachePath());
            AYPath::Resolver::addSearchPath(AYPath::Engine::getPresetModelPath());
            AYPath::Resolver::addSearchPath(AYPath::Engine::getPresetAudioPath());
            AYPath::Resolver::addSearchPath(AYPath::Engine::getPresetVideoPath());
            AYPath::Resolver::addSearchPath(AYPath::Engine::getPresetTexturePath());
            AYPath::Resolver::addSearchPath(AYPath::Engine::getPresetShaderPath());
            AYPath::Resolver::addSearchPath(AYPath::Engine::getPresetConfigPath());

            // 设置路径别名

            AYPath::Resolver::setAlias("@models", AYPath::Engine::getPresetModelPath());
            AYPath::Resolver::setAlias("@shaders", AYPath::Engine::getPresetShaderPath());
            AYPath::Resolver::setAlias("@audios", AYPath::Engine::getPresetAudioPath());
            AYPath::Resolver::setAlias("@videos", AYPath::Engine::getPresetVideoPath());
            AYPath::Resolver::setAlias("@textures", AYPath::Engine::getPresetTexturePath());
            AYPath::Resolver::setAlias("@web", AYPath::Engine::getPresetWebPath());
            AYPath::Resolver::setAlias("@config", AYPath::Engine::getPresetConfigPath());
            });
    }

    static std::string resolve(const std::string& path) {
        init();
        return Resolver::resolve(path);
    }
public:
    class Resolver
    {
    public:
        // 添加搜索路径
        static void addSearchPath(const std::string& path);

        // 设置路径别名
        static void setAlias(const std::string& alias, const std::string& path);

        // 智能解析路径
        static std::string resolve(const std::string& inputPath);

        // 检查文件是否存在
        static bool fileExists(const std::string& path);

        // 设置默认资源根路径
        static void setAssetsRoot(const std::string& rootPath);

    private:
        static std::vector<std::string> _searchPaths;
        static std::unordered_map<std::string, std::string> _aliases;
        static std::unordered_map<std::string, std::string> _resolvedCache;

        static std::mutex _mutex;
        static std::string _assetsRoot;

        // 内部路径拼接方法
        static std::string _combine(const std::string& base, const std::string& relative);

        // 规范化路径
        static std::string _normalize(const std::string& path);
    };

    class Engine
    {
    public:
        static std::string getAssetsPath();
        static std::string getPresetPath();
        static std::string getUserPath();
        static std::string getPresetTexturePath();
        static std::string getPresetModelPath();
        static std::string getPresetShaderPath();
        static std::string getPresetConfigPath();
        static std::string getPresetCachePath();
        static std::string getPresetAudioPath();
        static std::string getPresetVideoPath();
        static std::string getPresetWebPath();

    private:
        // 静态路径成员变量
        static const std::string AssetsPath;
        static const std::string PresetPath;
        static const std::string UserPath;
        static const std::string PresetTexturePath;
        static const std::string PresetModelPath;
        static const std::string PresetCachePath;
        static const std::string PresetShaderPath;
        static const std::string PresetConfigPath;
        static const std::string PresetAudioPath;
        static const std::string PresetVideoPath;
        static const std::string PresetWebPath;
    };

    class Windows
    {
    public:
        static std::string getAppCachePath();
        static std::string getRomingPath();
        static std::string getTempPath();
        static std::string getDesktopPath();
        static std::string getDocumentsPath();
        static std::string getPicturesPath();
        static std::string getMusicPath();
        static std::string getVideosPath();
        static std::string getFontsPath();

    private:
        static const std::string AppName;
    };
};