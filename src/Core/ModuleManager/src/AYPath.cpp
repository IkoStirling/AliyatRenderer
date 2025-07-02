#include "AYPath.h"

const std::string& AppName = std::string("AliyatRenderer");
const std::string& AssetsPath = std::string("assets/");
const std::string& PresetPath = AssetsPath + std::string("core/");
const std::string& PresetTexturePath = PresetPath + std::string("textures/");
const std::string& PresetCachePath = PresetPath + std::string("cache/");
const std::string& PresetShaderPath = PresetPath + std::string("shaders/");
const std::string& PresetConfigPath = PresetPath + std::string("config/");

const std::string& AYPath::Engine::getAssetsPath() { return AssetsPath; }

const std::string& AYPath::Engine::getPresetTexturePath() { return PresetTexturePath; }

const std::string& AYPath::Engine::getPresetShaderPath()
{
	return PresetShaderPath;
}

const std::string& AYPath::Engine::getPresetConfigPath()
{
	return PresetConfigPath;
}


const std::string& AYPath::Windows::getAppCachePath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))	//HWND用于可能的用户交互
	{
		return std::string(path) + AppName + "/cache/";
	}
	return PresetCachePath;
}

const std::string& AYPath::Windows::getRomingPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return PresetCachePath;
}

const std::string& AYPath::Windows::getTempPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return PresetCachePath;
}

const std::string& AYPath::Windows::getDesktopPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return "";
}

const std::string& AYPath::Windows::getDocumentsPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return PresetPath + "documents/";
}

const std::string& AYPath::Windows::getPicturesPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_PICTURES, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return PresetPath + "pictures/";
}

const std::string& AYPath::Windows::getMusicPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_MUSIC, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return PresetPath + "music/";
}

const std::string& AYPath::Windows::getVideosPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_VIDEO, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return PresetPath + "videos/";
}

const std::string& AYPath::Windows::getFontsPath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, path)))
	{
		return std::string(path) + AppName + "/";
	}
	return PresetPath + "font/";
}
	
