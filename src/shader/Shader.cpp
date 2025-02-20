#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* v_path, const char* f_path) : 
	shaderProgram(0),
	vertexShader(0),
	fragmentShader(0),
	errorInfoLogBuffer(new char[errorInfoLogBufferSize])
{
	std::string vertexCode, fragmentCode;
	std::ifstream vShaderFile, fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vShaderFile.open(v_path);
		fShaderFile.open(f_path);

		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		const char* vertexShaderSource = vertexCode.c_str();
		const char* fragmentShaderSource = fragmentCode.c_str();

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

		char infolog[1024];
		int success = 0;

		glCompileShader(vertexShader);
		checkShaderErrors(vertexShader, "COMPILE");

		glCompileShader(fragmentShader);
		checkShaderErrors(fragmentShader, "COMPILE");

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);

		glLinkProgram(shaderProgram);
		checkShaderErrors(shaderProgram, "LINK");

		setConstUniform();
	}
	catch (std::ifstream::failure &e)
	{
		std::cout << "shader source file error : " << e.what() << std::endl;
	}

}

Shader::~Shader()
{
	delete[] errorInfoLogBuffer;

}

void Shader::start()
{
	glUseProgram(shaderProgram);
}

void Shader::shutdown()
{
	glUseProgram(0);
}

void Shader::setTime()
{
	GLint location = glGetUniformLocation(shaderProgram, "time");
	glUniform1f(location, glfwGetTime());
	std::cout << glfwGetTime() << std::endl;
}

void Shader::setConstUniform()
{
	start();
	setUniform1f("PI", 3.141592); 


	shutdown();
}

void Shader::setUniform1f(const std::string& type_name, float value)
{
	GLint location = glGetUniformLocation(shaderProgram, type_name.c_str());
	glUniform1f(location, value);
}

void Shader::setUniform1i(const std::string& type_name, int value)
{
	GLint location = glGetUniformLocation(shaderProgram, type_name.c_str());
	glUniform1f(location, value);
}

void Shader::setUniform2f(const std::string& type_name, float value1, float value2)
{
	GLint location = glGetUniformLocation(shaderProgram, type_name.c_str());
	glUniform2f(location, value1, value2);
}

void Shader::setUniformMatrix4fv(const std::string& type_name, float* value)
{
	GLint location = glGetUniformLocation(shaderProgram, type_name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::setVertexShader(const char& v_path)
{
}

bool Shader::checkShaderErrors(GLuint target, const std::string& type)
{
	checkStatus = GL_TRUE;
	memset(errorInfoLogBuffer, 0, errorInfoLogBufferSize);

	if (type == "COMPILE")
	{
		glGetShaderiv(target, GL_COMPILE_STATUS, &checkStatus);
		if (checkStatus == GL_FALSE)
		{
			glGetShaderInfoLog(target, errorInfoLogBufferSize, NULL, errorInfoLogBuffer);
			std::cout << "shader compile error : " << errorInfoLogBuffer << std::endl;
			return false;
		}
	}
	else if (type == "LINK")
	{
		glGetShaderiv(target, GL_LINK_STATUS, &checkStatus);
		if (checkStatus == GL_FALSE)
		{
			glGetShaderInfoLog(target, errorInfoLogBufferSize, NULL, errorInfoLogBuffer);
			std::cout << "shader link error : " << errorInfoLogBuffer << std::endl;
			return false;
		}
	}
	

}


