#pragma once
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include <filesystem>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>

class AYShaderManager
{
public:
	AYShaderManager() = default;

public:
	struct ShaderInfo
	{
		GLuint shaderProgram;
		bgfx::ProgramHandle shaderProgramB;
		std::string vertexShaderPath;
		std::string fragmentShaderPath;
		std::filesystem::file_time_type vertexLastWrite;
		std::filesystem::file_time_type fragmentLastWrite;
	};

	void shaderFileWatch(float delta_time)
	{
		// 3 秒检查一次
		static float time = 0;
		time += delta_time;
		if (time > 3.f)
		{
			time -= 3.f;
			for (auto& [name, info] : _shaders) {
				auto vtime = std::filesystem::last_write_time(info.vertexShaderPath);
				auto ftime = std::filesystem::last_write_time(info.fragmentShaderPath);
				if (vtime != info.vertexLastWrite || ftime != info.fragmentLastWrite) {
					reloadShader(name);
					info.vertexLastWrite = vtime;
					info.fragmentLastWrite = ftime;
				}
			}
		}

	}

	bgfx::ProgramHandle loadShaderB(const std::string& name, const std::string& vertex_shaderPath, const std::string& fragment_shaderPath, bool reload = false)
	{
		// 检查是否需重载
		if (!reload) {
			auto it = _shaders.find(name);
			if (it != _shaders.end()) {
				return it->second.shaderProgramB;
			}
		}

		// 销毁旧程序（如果存在）
		auto it = _shaders.find(name);
		if (it != _shaders.end() && bgfx::isValid(it->second.shaderProgramB)) {
			bgfx::destroy(it->second.shaderProgramB);
		}

		// 加载着色器
		std::string nameWithTime = name + "_" + std::to_string(std::time(nullptr));
		bgfx::ShaderHandle vsh = loadBgfxShader(vertex_shaderPath, (nameWithTime +"_VS").c_str());
		bgfx::ShaderHandle fsh = loadBgfxShader(fragment_shaderPath, (nameWithTime + "_VS").c_str());
		if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh)) {
			if (bgfx::isValid(vsh)) bgfx::destroy(vsh);
			if (bgfx::isValid(fsh)) bgfx::destroy(fsh);
			return BGFX_INVALID_HANDLE;
		}

		// 创建程序（不自动销毁着色器，便于热重载）
		bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, false);

		// 保存信息
		ShaderInfo info;
		info.shaderProgramB = program;
		info.vertexShaderPath = vertex_shaderPath;
		info.fragmentShaderPath = fragment_shaderPath;
		_shaders[name] = info;

		return program;
	}

	GLuint loadShader(const std::string& name, const std::string& vertex_shaderPath, const std::string& fragment_shaderPath, bool reload = false)
	{
		if (!reload)
		{
			auto it = _shaders.find(name);
			if (it != _shaders.end())
				return it->second.shaderProgram;
		}


		std::string vertexCode, fragmentCode;
		std::ifstream vShaderFile, fShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			vShaderFile.open(vertex_shaderPath);
			fShaderFile.open(fragment_shaderPath);
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			vShaderFile.close();
			fShaderFile.close();
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			const char* vertexShaderSource = vertexCode.c_str();
			const char* fragmentShaderSource = fragmentCode.c_str();


			GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
			GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

			glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

			// 检查 Shader 是否编译成功
			GLint success;
			GLchar infoLog[512];

			glCompileShader(vertexShader);

			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
				std::cerr << "Vertex Shader Error: " << infoLog << std::endl;
			}

			glCompileShader(fragmentShader);

			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
				std::cerr << "Fragment Shader Error: " << infoLog << std::endl;
			}

			GLuint shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, vertexShader);
			glAttachShader(shaderProgram, fragmentShader);

			glLinkProgram(shaderProgram);

			// 检查 Shader 是否链接成功
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
				std::cerr << "Shader Link Error: " << infoLog << std::endl;
			}

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			_shaders.insert_or_assign(name, ShaderInfo{ shaderProgram, 0,  vertex_shaderPath, fragment_shaderPath });
			return shaderProgram;
		}
		catch (std::ifstream::failure& e)
		{
			std::cout << "shader source file error : " << e.what() << std::endl;
			return  0;
		}
	}

	void reloadAllShaders()
	{
		for (auto& [name, info] : _shaders) {
			if (bgfx::isValid(info.shaderProgramB)) {
				bgfx::destroy(info.shaderProgramB);
				loadShaderB(name, info.vertexShaderPath, info.fragmentShaderPath, true);
			}
			if (info.shaderProgram != 0) {
				glDeleteProgram(info.shaderProgram);
				loadShader(name, info.vertexShaderPath, info.fragmentShaderPath, true);
			}
		}
	}

	void reloadShader(const std::string& name)
	{
		auto it = _shaders.find(name);
		if (it == _shaders.end()) return;

		ShaderInfo& info = it->second;
		if (bgfx::isValid(info.shaderProgramB)) {
			bgfx::destroy(info.shaderProgramB);
			info.shaderProgramB = loadShaderB(name, info.vertexShaderPath, info.fragmentShaderPath, true);
		}
		if (info.shaderProgram != 0) {
			glDeleteProgram(info.shaderProgram);
			info.shaderProgram = loadShader(name, info.vertexShaderPath, info.fragmentShaderPath, true);
		}
	}
private:
	std::unordered_map<std::string, ShaderInfo> _shaders;
	bgfx::ShaderHandle loadBgfxShader(const std::string& path, const char* name) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			spdlog::error("Failed to open shader file: {}", path);
			return BGFX_INVALID_HANDLE;
		}

		// 读取文件大小
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);

		// 分配内存并读取
		const bgfx::Memory* mem = bgfx::alloc(size + 1);
		file.read((char*)mem->data, size);
		mem->data[size] = '\0'; // 添加终止符
		file.close();

		// 创建着色器
		bgfx::ShaderHandle handle = bgfx::createShader(mem);
		if (!bgfx::isValid(handle)) {
			spdlog::error("Failed to compile shader: {}", path);
			return BGFX_INVALID_HANDLE;
		}

		bgfx::setName(handle, name);
		return handle;
	}
};