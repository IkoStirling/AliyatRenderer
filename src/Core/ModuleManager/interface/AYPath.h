#pragma once
#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>
#include <string>

class AYPath
{
public:
    AYPath();

public:
    class Engine
    {
    public:
        static std::string getAssetsPath();
        static std::string getPresetPath();
        static std::string getPresetTexturePath();
        static std::string getPresetShaderPath();
        static std::string getPresetConfigPath();
        static std::string getPresetCachePath();
        static std::string getPresetAudioPath();
        static std::string getPresetVideoPath();

    private:
        // 静态路径成员变量
        static const std::string AssetsPath;
        static const std::string PresetPath;
        static const std::string PresetTexturePath;
        static const std::string PresetCachePath;
        static const std::string PresetShaderPath;
        static const std::string PresetConfigPath;
        static const std::string PresetAudioPath;
        static const std::string PresetVideoPath;
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