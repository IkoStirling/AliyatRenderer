#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "checkError.h"
#include "Application.h"

#include <iostream>

#define numVbo 3
#define numEbo 1
#define numVao 1

GLuint vao;


void OnResize(int width, int height) 
{
	glViewport(0, 0, width, height);
	//std::cout << "do nothing" << std::endl;
}

void KeyHandle(int key, int action, int mods) 
{
	/*
		action == 
			GLFW_PRESS
			GLFW_RELEASE
		mods ==
			GLFW_MOD_CTRL
			GLFW_MOD_SHIFT
	*/
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			std::cout << "W" << std::endl;
			break;
		case GLFW_KEY_A:
			std::cout << "A" << std::endl;
			break;
		case GLFW_KEY_S:
			std::cout << "S" << std::endl;
			break;
		case GLFW_KEY_D:
			std::cout << "D" << std::endl;
			break;
		default:
			break;
		}
	}
}

GLuint test_draw_color_triangle()
{
	float vertices[] =
	{
		-0.5f, -0.5f, 0.f,
		0.5f, -0.5f, 0.f,
		0.f, 0.5f, 0.f,
		0.5f, 0.5f, 0.f,
		0.8f, 0.8f, 0.f,
		-0.8f, 0.8f, 0.f,
	};
	float colors[] =
	{
		1.f, 0.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 0.f, 1.f
	};
	//unsigned int indices[] =
	//{
	//	0,1,2,
	//	2,3,1,
	//	6,5,4
	//};
	const char* vshaderSource =
		"#version 460 \n"
		"out vec3 vColor; \n"
		"layout(location=0) in vec3 aPos; \n"
		"layout(location=1) in vec3 aColor; \n"
		"out vec3 vPos; \n"
		"void main(void) \n"
		"{ \n "
		"gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0); \n"
		"vColor = aColor; \n"
		"vPos = gl_Position.xyz; \n"
		"}";
	const char* fshaderSource =
		"#version 460 \n"
		"in vec3 vColor; \n"
		"in vec3 vPos; \n"
		"out vec4 color; \n"
		"void main(void) \n"
		"{color = vec4(smoothstep(0.,1.,vPos+0.5),1.0);}";

	GLuint vbos[numVbo];
	//GLuint ebo;
	glGenBuffers(numVbo, vbos);
	//glGenBuffers(numEbo, &ebo);
	glGenVertexArrays(numVao, &vao);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, *(vbos + 1));
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

	//glBindVertexArray(0);

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vshader, 1, &vshaderSource, NULL);
	glShaderSource(fshader, 1, &fshaderSource, NULL);

	glCompileShader(vshader);
	glCompileShader(fshader);

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vshader);
	glAttachShader(vfProgram, fshader);

	glLinkProgram(vfProgram);

	/*glDeleteBuffers(numVbo, vbos);
	glDeleteVertexArrays(numVao, &vao);*/

	return vfProgram;
}



int main() {
	
	if (!app->init(1080, 720)) return -1;


	//GLuint vfshaderP = test_draw_color_triangle();
	

	glViewport(0, 0, 1080, 720);
	glClearColor(0.2f, 0.3f, 0.2f, 1.0f);

	app->setFrameBufferResizeCallBack(OnResize);
	app->setKeyCallBack(KeyHandle);

	while (app->update())
	{

		glClear(GL_COLOR_BUFFER_BIT);
		//renderingProgram();
		//glUseProgram(vfshaderP);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	app->destory();
	

	return 0;
}