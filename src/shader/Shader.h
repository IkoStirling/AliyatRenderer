#pragma once
#include "core/core.h"
#include <string>

#define errorInfoLogBufferSize 1024
class Shader
{
public:
	Shader(const char* v_path, const char* f_path);
	~Shader();

	void start();
	void shutdown();
	void setTime();
	void setConstUniform();
	void setUniform1f(const std::string& type_name, float value);
	void setUniform1i(const std::string& type_name, int value);
	void setUniform2f(const std::string& type_name, float value1, float value2);
	void setUniformMatrix4fv(const std::string& type_name, float* value);
	void setVertexShader(const char& v_path);
	GLuint shaderProgram;
private:
	
	GLuint vertexShader;
	GLuint fragmentShader;

private:
	char* errorInfoLogBuffer;
	GLint checkStatus;
	bool checkShaderErrors(GLuint target, const std::string& type);

};