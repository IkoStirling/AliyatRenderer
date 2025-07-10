#pragma once
#include <vector>
#include "glm/glm.hpp"

class AYGraphicGenerator
{
public:
	// ----------------Vertex------------------
	static std::vector<glm::vec3> createCircleV(float radius, int segments);
	static std::vector<glm::vec3> createBoxV(const glm::vec3& half_extents);
	static std::vector<glm::vec3> createRectV();


public:
	// ----------------Indices-----------------
	static std::vector<uint32_t> createCircleI(bool filled, int segments);
	static std::vector<uint32_t> createBoxI(bool wireframe);
	static std::vector<uint32_t> createRectI(bool filled);

};