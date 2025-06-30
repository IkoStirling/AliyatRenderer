#pragma once
#include <Windows.h>	//精力有限，无法兼容linux，后续可考虑
#include <ShlObj.h> 
#include <string>

namespace Path
{
	const std::string& AppName = std::string("AliyatRenderer");
	const std::string& AssetsPath = std::string("assets/");
	const std::string& PresetPath = AssetsPath + std::string("core/");
	const std::string& PresetTexturePath = PresetPath + std::string("textures/");
	const std::string& PresetCachePath = PresetPath + std::string("cache/");

	namespace Windows
	{
		const std::string& getAppCachePath();
		const std::string& getRomingPath();
		const std::string& getTempPath();
		const std::string& getDesktopPath();
		const std::string& getDocumentsPath();
		const std::string& getPicturesPath();
		const std::string& getMusicPath();
		const std::string& getMusicPath();
		const std::string& getVideosPath();
		const std::string& getFontsPath();
	}

	namespace Engine
	{
		const std::string& getAssetsPath();
		const std::string& getPresetTexturePath();

	}

	namespace Windows
	{
		inline const std::string& getAppCachePath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))	//HWND用于可能的用户交互
			{
				return std::string(path) + AppName + "/cache/";
			}
			return PresetCachePath;
		}

		inline const std::string& getRomingPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return PresetCachePath;
		}

		inline const std::string& getTempPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return PresetCachePath;
		}

		inline const std::string& getDesktopPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return "";
		}

		inline const std::string& getDocumentsPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return PresetPath + "documents/";
		}

		inline const std::string& getPicturesPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_PICTURES, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return PresetPath + "pictures/";
		}

		inline const std::string& getMusicPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_MUSIC, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return PresetPath + "music/";
		}

		inline const std::string& getVideosPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_VIDEO, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return PresetPath + "videos/";
		}

		inline const std::string& getFontsPath()
		{
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, path)))
			{
				return std::string(path) + AppName + "/";
			}
			return PresetPath + "font/";
		}
	}

	namespace Engine
	{
		inline const std::string& getAssetsPath() { return AssetsPath; }

		inline const std::string& getPresetTexturePath() { return PresetTexturePath; }
	}



}

