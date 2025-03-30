#include "core/core.h"
#include "application/Application.h"
#include "shader/Shader.h"
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "texture/Texture.h"
#include "camera/PerspectiveCamera.h"
#include "camera/OrthographicCamera.h"
#include "object/World.h"
#include "pool/Pool.h"


#define numVbo 3
#define numEbo 2
#define numVao 2
#define numFbo 1

Shader* shader;
Shader* screenShader;
Camera* camera;
glm::mat4 proj_matrix, view_matrix, model_matrix, mv_mat;
Pool* poolVAO, * poolVBO, * poolEBO;


void OnResize(int width, int height)
{
	glViewport(0, 0, width, height);
	//std::cout << "do nothing" << std::endl;
}
bool keyW, keyA, keyS, keyD, keyShift;
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
	if (key == GLFW_KEY_W)
		keyW = (action == GLFW_PRESS || action == GLFW_REPEAT);
	if (key == GLFW_KEY_A)
		keyA = (action == GLFW_PRESS || action == GLFW_REPEAT);
	if (key == GLFW_KEY_S)
		keyS = (action == GLFW_PRESS || action == GLFW_REPEAT);
	if (key == GLFW_KEY_D)
		keyD = (action == GLFW_PRESS || action == GLFW_REPEAT);
	if (key == GLFW_KEY_LEFT_SHIFT)
		keyShift = (action == GLFW_PRESS || action == GLFW_REPEAT);
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(app.getWindowInstance(),true);
}

int centerX, centerY;


void MouseMove(double xpos, double ypos)
{
	camera->setCameraYawPitchView(xpos, ypos);
}
void test_draw_color_triangle()
{
	unsigned int indices[] =
	{
		0,1,2,
		2,1,3
	};
	float vertices[] =
	{
		-0.5f, -0.5f, 0.f,
		0.5f, -0.5f, 0.f,
		-0.5f, 0.5f, 0.f,
		0.5f, 0.5f, 0.f,
	};
	float colors[] =
	{
		1.f, 0.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 0.f, 1.f
	};

	float uvs[] = {
		0.f,0.f,
		1.f,0.f,
		0.f,1.f,
		1.f,1.f,
	};

	GLuint vbos[numVbo];
	GLuint ebo;
	GLuint vao;
	glGenBuffers(numVbo, vbos);
	glGenBuffers(numEbo, &ebo);
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

	glBindBuffer(GL_ARRAY_BUFFER, *(vbos + 2));
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	shader = new Shader(
		"assets/core/shaders/vertexShader.glsl",
		"assets/core/shaders/fragmentShader.glsl"
	);
}
GLuint vbos[numVbo];
GLuint vaos[numVao];
GLuint ebos[numEbo];
GLuint fbos[numFbo];
GLuint texture1;
GLuint tex, rbo;


void test_cube()
{
	DEBUG_COLLECT();
	unsigned int indices[] =
	{
		0,2,1,
		1,2,3,
		1,7,5,
		1,3,7,
		0,5,4,
		0,1,5,
		3,6,7,
		3,2,6,
		0,4,6,
		0,6,2,
		5,6,4,
		5,7,6
	};
	float vertices[] =
	{
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
	};

	float screen[] =
	{
		-1.0f,-1.0f,0.f,	0.f,0.f,
		-1.0f,1.0f,0.f,		0.f,1.f,
		1.0f,-1.0f,0.f,		1.f,0.f,
		1.0f,1.0f,0.f,		1.f,1.f,
	};

	//float screen[] =
	//{
	//	-1.0f,-1.0f,0.f,
	//	-1.0f,1.0f,0.f,
	//	1.0f,-1.0f,0.f,
	//	1.0f,1.0f,0.f,
	//};

	unsigned int screenIndices[] =
	{
		0,2,1,
		1,2,3,
	};


	shader = new Shader(
		"assets/core/shaders/vertexShader.glsl",
		"assets/core/shaders/fragmentShader.glsl"
	);
	screenShader = new Shader(
		"assets/core/shaders/screenSpace.vert",
		"assets/core/shaders/screenSpace.frag"
	);

	glGenBuffers(numVbo, vbos);
	glGenBuffers(numEbo, ebos);
	glGenBuffers(numFbo, fbos);
	glGenTextures(1, &tex);
	glGenVertexArrays(numVao, vaos);

	glBindVertexArray(*vaos);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1080, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);





	glBindBuffer(GL_ARRAY_BUFFER, *vbos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen), screen, GL_STATIC_DRAW);

	GLuint screenV = glGetAttribLocation(screenShader->shaderProgram, "screenVertices");
	if (screenV == -1)
		std::cerr << "wwwww-------------------------1" << std::endl;
	glEnableVertexAttribArray(screenV);
	glVertexAttribPointer(screenV, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	//glVertexAttribPointer(screenV, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	GLuint screenUV = glGetAttribLocation(screenShader->shaderProgram, "inUV");
	if (screenUV == -1)
		std::cerr << "wwwww-------------------------2" << std::endl;
	glEnableVertexAttribArray(screenUV);
	glVertexAttribPointer(screenUV, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebos);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenIndices), screenIndices, GL_STATIC_DRAW);






	glBindVertexArray(*vaos + 1);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos + 1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	GLuint aPos = glGetAttribLocation(shader->shaderProgram, "aPos");
	if (aPos == -1)
		std::cerr << "wwwww-------------------------3" << std::endl;
	glEnableVertexAttribArray(aPos);
	glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(ebos + 1));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	proj_matrix = camera->getProjectionMatrix();
	shader->setUniformMatrix4fv("proj_matrix", glm::value_ptr(proj_matrix));
	camera->setCameraWorldPosition({ 0.0f,1.7f,10.f });
	shader->setUniform2f("iResolution", 1080.f, 720.f);
}

void render()
{

	shader->start();
	glBindVertexArray(*vaos + 1);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbos);
	/*glBindBuffer(GL_ARRAY_BUFFER, *(vbos + 1));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(ebos+1));*/

	//GLuint aPos = glGetAttribLocation(shader->shaderProgram, "aPos");
	//glEnableVertexAttribArray(aPos);
	//glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	shader->setUniform1f("time", glfwGetTime());
	shader->setUniform1i("texture1", 0);

	view_matrix = camera->getViewMatrix();
	model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -2.f, 0.f));
	mv_mat = view_matrix * model_matrix;

	shader->setUniformMatrix4fv("mv_matrix", glm::value_ptr(mv_mat));
	shader->setUniformMatrix4fv("proj_matrix", glm::value_ptr(proj_matrix));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)), 10);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	shader->shutdown();


	screenShader->start();
	glBindVertexArray(*vaos);
	//glBindBuffer(GL_ARRAY_BUFFER, *vbos);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebos);
	//GLuint screenV = glGetAttribLocation(screenShader->shaderProgram, "screenVertices");
	//glEnableVertexAttribArray(screenV);
	//glVertexAttribPointer(screenV, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, tex);
	//screenShader->setUniform1i("texture1", 0);
	//glDisable(GL_DEPTH_TEST);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
	screenShader->shutdown();
}

#include <boost/asio.hpp>
#include "shape/Model.h"
void postTest(int seconds)
{
	DEBUG_COLLECT();
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
#include "scene/BaseSceneWithCheckerboard.h"


void keyEventHandle(double deltaTime)
{
	float speed;
	if (keyShift)
		speed = 20.f;
	else
		speed = 10.f;


	if (keyW)
		camera->move(camera->front * speed * static_cast<float>(deltaTime));
	if (keyA)
		camera->move(-camera->right * speed * static_cast<float>(deltaTime));
	if (keyS)
		camera->move(-camera->front * speed * static_cast<float>(deltaTime));
	if (keyD)
		camera->move(camera->right * speed * static_cast<float>(deltaTime));

}
int main() {

	/*postTest(3);
	postTest(6);
	postTest(1);*/
	
	//getchar();

	if (!app.init(1080, 720)) return -1;

	poolVAO = new Pool(100);
	poolVBO = new Pool(1000);
	poolEBO = new Pool(100);


	camera = new PerspectiveCamera(getWorld(), 90, 1080.f / 720.f, 0.1f, 1000.f);
	test_cube();
	DEBUG_CONSOLE_SHOW(test_cube);
	//getchar();

	Texture* checkerboard = new Texture("assets/core/textures/checkerboard.png", 0);
	BaseSceneWithCheckerboard* baseScene = new BaseSceneWithCheckerboard(camera, poolVAO, poolVBO, poolEBO);
	baseScene->setTexture(checkerboard);
	baseScene->presetData();


	glViewport(0, 0, 1080, 720);
	glClearColor(0.f, 0.f, 0.f, 0.7f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	app.setFrameBufferResizeCallBack(OnResize);
	app.setKeyCallBack(KeyHandle);
	app.setCursorPosCallback(MouseMove);

	glfwSetInputMode(app.getWindowInstance(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	double lastTime = glfwGetTime();
	while (app.update())
	{
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;
		//if (deltaTime < (1.0 / 60.0))
		//{
		//	double sleepTime = (1.0 / 60.0) - deltaTime;
		//	glfwWaitEventsTimeout(sleepTime);
		//	continue;
		//}
		lastTime = currentTime;
		//getWorld()->move({0.f,1.f * glm::sin(glfwGetTime()),0.f});
		keyEventHandle(deltaTime);

		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		baseScene->render();
		render();
	}

	app.destory();

	//DEBUG_CONSOLE_SHOW(render);
	return 0;
}