#pragma once
#include "GLFW/glfw3.h"
#include "glad/glad.h"
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
		std::string vertexShaderPath;
		std::string fragmentShaderPath;
	};

	GLuint loadShader(const std::string& name, const std::string& vertex_shaderPath, const std::string& fragment_shaderPath, bool reload = false)
	{
		if(!reload)
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

			_shaders.insert_or_assign(name, ShaderInfo{ shaderProgram, vertex_shaderPath, fragment_shaderPath });
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
		for (auto& it : _shaders)
		{
			reloadShader(it.first);
		}
	}

	void reloadShader(const std::string& name)
	{
		auto it = _shaders.find(name);
		if (it == _shaders.end())
			return;
		auto& shader = it->second;
		glDeleteProgram(shader.shaderProgram);
		loadShader(name, shader.vertexShaderPath, shader.fragmentShaderPath, true);
	}
private:
	std::unordered_map<std::string, ShaderInfo> _shaders;
};