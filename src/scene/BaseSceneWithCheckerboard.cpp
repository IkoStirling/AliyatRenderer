#include "BaseSceneWithCheckerboard.h"
#include "pool/Pool.h"
#include "shader/Shader.h"
#include "camera/Camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include "texture/SkyBoxTexture.h"
#include "shape/Model.h"

BaseSceneWithCheckerboard::BaseSceneWithCheckerboard(Camera* _camera,Pool* _poolVAO, Pool* _poolVBO, Pool* _poolEBO):
	Scene(_poolVAO, _poolVBO, _poolEBO),
	camera(_camera),
	texture(nullptr),
	model_matrix(0),
	proj_matrix(0),
	view_matrix(0),
	mv_mat(0)
{
	shader = new Shader(
		"assets/core/shaders/baseSceneWithCheckerboard.vert",
		"assets/core/shaders/baseSceneWithCheckerboard.frag"
	);
	sky = new SkyBoxTexture("assets/core/textures/skyBox.png", 1);
	presetSkyBox();
}

BaseSceneWithCheckerboard::~BaseSceneWithCheckerboard()
{
}

void BaseSceneWithCheckerboard::presetData()
{
	if (!poolVAO || !poolVBO || !poolEBO)
		return;
	vao = poolVAO->getUnusedElement();
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	for (int i = 0; i < 3; i++)
	{
		GLuint vbo = poolVBO->getUnusedElement();
		glGenBuffers(1, &vbo);
		vbos.push_back(vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);
	GLuint inPosition = glGetAttribLocation(shader->shaderProgram, "inPosition");
	if (inPosition == -1)
		std::cerr << "wwwww-------------"<< PARAM_TO_STR(inPosition) << "------------" << std::endl;
	glEnableVertexAttribArray(inPosition);
	glVertexAttribPointer(inPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_texcoords), square_texcoords, GL_STATIC_DRAW);
	GLuint inTexcoords = glGetAttribLocation(shader->shaderProgram, "inTexcoords");
	if (inTexcoords == -1)
		std::cerr << "wwwww-------------" << PARAM_TO_STR(inTexcoords) << "------------" << std::endl;
	glEnableVertexAttribArray(inTexcoords);
	glVertexAttribPointer(inTexcoords, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_texcoords), square_texcoords, GL_STATIC_DRAW);
	GLuint inNormal = glGetAttribLocation(shader->shaderProgram, "inNormal");
	if (inNormal == -1)
		std::cerr << "wwwww-------------" << PARAM_TO_STR(inNormal) << "------------" << std::endl;
	glEnableVertexAttribArray(inNormal);
	glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	for (int i = 0; i < 1; i++)
	{
		GLuint ebo = poolEBO->getUnusedElement();
		glGenBuffers(1, &ebo);
		ebos.push_back(ebo);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebos[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);

	glBindVertexArray(0);



}

void BaseSceneWithCheckerboard::render()
{
	DEBUG_COLLECT();
	skyBoxRender(sky->getCubemapTextureID());

	shader->start();
	glBindVertexArray(vao);
	glm::vec3 scale = { 200.f,200.f,1.f };
	model_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.f), glm::vec3(1.0f, 0.0f, 0.0f));
	proj_matrix = camera->getProjectionMatrix();
	shader->setUniformMatrix4fv("proj_matrix", glm::value_ptr(proj_matrix));
	shader->setUniform3f("m_scale", scale);
	shader->setUniform1f("shininess", 9.f);
	shader->setUniform1f("abientStrength", 0.9f);

	view_matrix = camera->getViewMatrix();
	shader->setUniformMatrix4fv("model", glm::value_ptr(model_matrix));
	shader->setUniformMatrix4fv("view", glm::value_ptr(view_matrix));
	shader->setUniform3f("cameraPos", camera->getCameraWorldPosition());
	shader->setUniform3f("lightDir", { glm::cos(glfwGetTime() / (2 * glm::pi<float>())),glm::sin(glfwGetTime()/(2*glm::pi<float>())),0.f });
	float c = (2.f - glm::abs(glm::cos(glfwGetTime() / (2 * glm::pi<double>())))) / 2.f;
	shader->setUniform3f("lightColor", { c,c,c });

	
	if(texture)
		texture->bindTexture();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

	glBindVertexArray(0);
	shader->shutdown();
}

void BaseSceneWithCheckerboard::skyBoxRender(GLuint cubemapTexture)
{
	shader->start();

	glm::vec3 scale = { 1.f,1.f,1.f };
	model_matrix = glm::mat4(1.0f);
	proj_matrix = glm::mat4(1.0f);
	shader->start();
	shader->setUniformMatrix4fv("proj_matrix", glm::value_ptr(proj_matrix));
	shader->setUniform3f("m_scale", scale);
	shader->setUniform1f("shininess", 9.f);
	shader->setUniform1f("abientStrength", 0.9f);
	shader->shutdown();

	glBindVertexArray(sky_vao);
	glDepthMask(GL_FALSE); // 禁用深度写入
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);


	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
	shader->shutdown();
}

void BaseSceneWithCheckerboard::presetSkyBox()
{
	if (!poolVAO || !poolVBO || !poolEBO)
		return;
	sky_vao = poolVAO->getUnusedElement();
	glGenVertexArrays(1, &sky_vao);
	glBindVertexArray(sky_vao);

	std::vector<Vertex> vertices;
	std::vector<TextureCoord> textureCoords;
	std::vector<Normal> normals;
	std::vector<Face> faces;
	Model::parseOBJ("assets/usrs/untitled.obj", vertices, textureCoords, normals, faces);

	for (int i = 0; i < 3; i++)
	{
		GLuint vbo = poolVBO->getUnusedElement();
		glGenBuffers(1, &vbo);
		sky_vbos.push_back(vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, sky_vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	GLuint inPosition = glGetAttribLocation(shader->shaderProgram, "inPosition");
	if (inPosition == -1)
		std::cerr << "sssss-------------" << PARAM_TO_STR(inPosition) << "------------" << std::endl;
	glEnableVertexAttribArray(inPosition);
	glVertexAttribPointer(inPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, sky_vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(TextureCoord), textureCoords.data(), GL_STATIC_DRAW);
	GLuint inTexcoords = glGetAttribLocation(shader->shaderProgram, "inTexcoords");
	if (inTexcoords == -1)
		std::cerr << "sssss-------------" << PARAM_TO_STR(inTexcoords) << "------------" << std::endl;
	glEnableVertexAttribArray(inTexcoords);
	glVertexAttribPointer(inTexcoords, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, sky_vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Normal), normals.data(), GL_STATIC_DRAW);
	GLuint inNormal = glGetAttribLocation(shader->shaderProgram, "inNormal");
	if (inNormal == -1)
		std::cerr << "sssss-------------" << PARAM_TO_STR(inNormal) << "------------" << std::endl;
	glEnableVertexAttribArray(inNormal);
	glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<unsigned int> indices;
	for (const auto& face : faces) {
		if (face.vertexIndices.size() == 4) {
			// 四边形面，分解为两个三角形
			indices.push_back(face.vertexIndices[0]);
			indices.push_back(face.vertexIndices[1]);
			indices.push_back(face.vertexIndices[2]);

			indices.push_back(face.vertexIndices[0]);
			indices.push_back(face.vertexIndices[2]);
			indices.push_back(face.vertexIndices[3]);
		}
		else if (face.vertexIndices.size() == 3) {
			// 三角形面，直接使用
			indices.push_back(face.vertexIndices[0]);
			indices.push_back(face.vertexIndices[1]);
			indices.push_back(face.vertexIndices[2]);
		}
	}
	for (int i = 0; i < 1; i++)
	{
		GLuint ebo = poolEBO->getUnusedElement();
		glGenBuffers(1, &ebo);
		sky_ebos.push_back(ebo);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sky_ebos[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);



}

void BaseSceneWithCheckerboard::setCamera(Camera* _camera)
{
	camera = _camera;
}

void BaseSceneWithCheckerboard::setTexture(Texture* _texture)
{
	texture = _texture;
}
