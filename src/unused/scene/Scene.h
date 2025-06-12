#pragma once
#include "core/core.h"
#include <vector>

class Shader;
class Pool;

class Scene
{
public:
	Scene(Pool* _poolVAO, Pool* _poolVBO, Pool* _poolEBO);
	virtual ~Scene();

	virtual void presetData() = 0;

	virtual void render() = 0;
	void setPoolVAO(Pool* _poolVAO);
	void setPoolVBO(Pool* _poolVBO);
	void setPoolEBO(Pool* _poolEBO);


protected:
	Shader* shader;
	Pool* poolVAO;
	Pool* poolVBO;
	Pool* poolEBO;
	GLuint vao;
	std::vector<GLuint> vbos;
	std::vector<GLuint> ebos;
};