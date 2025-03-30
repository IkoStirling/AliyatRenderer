#pragma once
#include "core/core.h"
#include "Scene.h"
#include "glm/glm.hpp"
class Camera;
class Shader;
class Texture;
class SkyBoxTexture;

class BaseSceneWithCheckerboard : public Scene
{
public:
	BaseSceneWithCheckerboard(Camera* _camera, Pool* _poolVAO, Pool* _poolVBO, Pool* _poolEBO);
	~BaseSceneWithCheckerboard();

	virtual void presetData()override;
	virtual void render()override;
	virtual void skyBoxRender(GLuint cubemapTexture);

	void presetSkyBox();

	void setCamera(Camera* _camera);
	void setTexture(Texture* _texture);

private:
	Camera* camera;
	Texture* texture;
	SkyBoxTexture* sky;
	glm::mat4 proj_matrix, view_matrix, model_matrix, mv_mat;
	GLuint sky_vao;
	std::vector<GLuint> sky_vbos;
	std::vector<GLuint> sky_ebos;
private:
	 float square_vertices[12]={
		-1.0f, -1.0f, 0.f,
		-1.0f, 1.0f, 0.f,
		1.0f, -1.0f, 0.f,
		1.0f, 1.0f, 0.f,
	 };
	 unsigned int square_indices[6] = { 
		0, 2, 1,
		1, 2, 3,
	 };
	 float square_texcoords[8] = {
		0.f, 0.f,
		0.f, 1.f,
		1.f, 0.f,
		1.f, 1.f,
	 };
	 float square_normal[12] = {
		 0.f,1.f,0.f,
		 0.f,1.f,0.f,
		 0.f,1.f,0.f,
		 0.f,1.f,0.f
	 };
};