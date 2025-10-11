#pragma once
#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <queue>
#include <mutex>

#include <bgfx/bgfx.h>
#include <bx/filepath.h>
#include <bx/file.h>
#include <bx/readerwriter.h>
#include <bimg/bimg.h>
#include <bx/bounds.h>
#include <bx/pixelformat.h>
#include <functional>

using BgfxUniformCallback = std::function<void(bgfx::ProgramHandle)>;
struct STShaderProgramB
{
	bgfx::ShaderHandle vsh;
	bgfx::ShaderHandle fsh;
	bgfx::ProgramHandle program;
	std::filesystem::file_time_type lastVModified;
	std::filesystem::file_time_type lastFModified;
	BgfxUniformCallback callback;
};

class AYBgfxCreator
{
public:
	void update(float delta_time);
	bgfx::ProgramHandle getProgram(const std::string& key);
	static const char* convertShaderPath(const bx::StringView& _name);
	void setUniformAndBind(const std::string& key, BgfxUniformCallback callback);
	
	static const bgfx::Memory* loadMem(const bx::FilePath& _filePath);
	static void* loadMem(const bx::FilePath& _filePath, uint32_t* _size);
	static bgfx::ShaderHandle loadShader(const bx::StringView& _name);
	bgfx::ProgramHandle loadProgram(const std::string& _vsName, const std::string& _fsName);
	static void imageReleaseCb(void* _ptr, void* _userData);
	bgfx::TextureHandle loadTexture(const bx::FilePath& _filePath, uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, uint8_t _skip = 0, bgfx::TextureInfo* _info = NULL, bimg::Orientation::Enum* _orientation = NULL);
	bimg::ImageContainer* imageLoad(const bx::FilePath& _filePath, bgfx::TextureFormat::Enum _dstFormat);
	void calcTangents(void* _vertices, uint16_t _numVertices, bgfx::VertexLayout _layout, const uint16_t* _indices, uint32_t _numIndices);

private:
	void* load(const bx::FilePath& _filePath, uint32_t* _size);
	void unload(void* _ptr);
	static bx::FileReader _reader;
	static bx::DefaultAllocator _defaultAllocator;

	std::unordered_map<std::string, STShaderProgramB> _shaderCache;
	std::queue<std::pair<std::string, STShaderProgramB>> _reloadQueue;
	std::mutex _reloadMutex;
	std::atomic<bool> _isReloading{ false };
};