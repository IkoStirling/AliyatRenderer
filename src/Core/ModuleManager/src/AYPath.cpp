#include "AYPath.h"

// 初始化静态成员变量
const std::string AYPath::Windows::AppName = "AliyatRenderer";

const std::string AYPath::Engine::AssetsPath = "assets/";
const std::string AYPath::Engine::PresetPath = AYPath::Engine::AssetsPath + "core/";
const std::string AYPath::Engine::PresetTexturePath = AYPath::Engine::PresetPath + "textures/";
const std::string AYPath::Engine::PresetCachePath = AYPath::Engine::PresetPath + "cache/";
const std::string AYPath::Engine::PresetShaderPath = AYPath::Engine::PresetPath + "shaders/";
const std::string AYPath::Engine::PresetConfigPath = AYPath::Engine::PresetPath + "config/";
const std::string AYPath::Engine::PresetAudioPath = AYPath::Engine::PresetPath + "audios/";
const std::string AYPath::Engine::PresetVideoPath = AYPath::Engine::PresetPath + "videos/";


std::string AYPath::Engine::getAssetsPath() { return AssetsPath; }
std::string AYPath::Engine::getPresetPath() { return PresetPath; }
std::string AYPath::Engine::getPresetTexturePath() { return PresetTexturePath; }
std::string AYPath::Engine::getPresetCachePath() { return PresetCachePath; }
std::string AYPath::Engine::getPresetShaderPath() { return PresetShaderPath; }
std::string AYPath::Engine::getPresetConfigPath() { return PresetConfigPath; }
std::string AYPath::Engine::getPresetAudioPath() { return PresetAudioPath; }
std::string AYPath::Engine::getPresetVideoPath() { return PresetVideoPath; }

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