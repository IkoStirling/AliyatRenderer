#pragma once
#include <vector>
#include "glm/glm.hpp"

struct VertexInfo
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	//glm::vec4 color;    //¶¥µãÑÕÉ«
};

class AYGraphicGenerator
{
public:
	// -------------VertexInfo----------------
	static std::vector<VertexInfo> createBox(const glm::vec3& half_extents, bool wireframe);


public:
	// ----------------Indices-----------------
	static std::vector<uint32_t> createBoxI(bool wireframe);
	static std::vector<uint32_t> createCircleI(bool filled, int segments);
	static std::vector<uint32_t> createRectI(bool filled);

public:
	// ----------------Vertex------------------
	static std::vector<glm::vec3> createCircleV(float radius, int segments);
	static std::vector<glm::vec3> createBoxV(const glm::vec3& half_extents);
	static std::vector<glm::vec3> createRectV();




public:
	// ----------------Normals-----------------
	static glm::vec3 create2DN();
	static std::vector<glm::vec3> createBoxN();

public:
	// -----------TextureCoordinate------------
	static std::vector<glm::vec2> createBoxV();
};