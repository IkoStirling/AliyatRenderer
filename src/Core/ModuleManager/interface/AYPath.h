#pragma once
#define NOMINMAX
#include <Windows.h>	//精力有限，无法兼容linux，后续可考虑

#include <ShlObj.h> 
#include <string>


class AYPath
{
public:
	AYPath();

private:


public:
	class Engine
	{
	public:
		static const std::string& getAssetsPath();
		static const std::string& getPresetTexturePath();
		static const std::string& getPresetShaderPath();
		static const std::string& getPresetConfigPath();
	};

	class Windows
	{
	public:
		static const std::string& getAppCachePath();
		static const std::string& getRomingPath();
		static const std::string& getTempPath();
		static const std::string& getDesktopPath();
		static const std::string& getDocumentsPath();
		static const std::string& getPicturesPath();
		static const std::string& getMusicPath();
		static const std::string& getVideosPath();
		static const std::string& getFontsPath();

	};

};
