#include "Scene.h"

Scene::Scene(Pool* _poolVAO, Pool* _poolVBO, Pool* _poolEBO):
	shader(nullptr),
	poolVAO(_poolVAO),
	poolVBO(_poolVBO),
	poolEBO(_poolEBO),
	vao(-1)
{

}

Scene::~Scene()
{
}

void Scene::setPoolVAO(Pool* _poolVAO)
{
	poolVAO = _poolVAO;
}

void Scene::setPoolVBO(Pool* _poolVBO)
{
	poolVBO = _poolVBO;
}

void Scene::setPoolEBO(Pool* _poolEBO)
{
	poolEBO = _poolEBO;
}
