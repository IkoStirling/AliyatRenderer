#include "BaseRendering/Graphic/AYGraphicGenerator.h"
#define M_PI 3.14159265f

std::vector<glm::vec3> AYGraphicGenerator::createCircleV(float radius, int segments)
{
	std::vector<glm::vec3> circleVertices;
	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		circleVertices.push_back(glm::vec3(
			cos(angle) * radius,
			sin(angle) * radius,
			0.0f
		));
	}
	return circleVertices;
}

std::vector<glm::vec3> AYGraphicGenerator::createBoxV(const glm::vec3& half_extents)
{
	std::vector<glm::vec3> boxVertices;
	boxVertices = {
		 { -half_extents.x, -half_extents.y, half_extents.z },
		 { half_extents.x, -half_extents.y, half_extents.z },
		 { half_extents.x, half_extents.y, half_extents.z },
		 { -half_extents.x, half_extents.y, half_extents.z },

		 // 后面（z=-half_extents.z）
		 { -half_extents.x, -half_extents.y, -half_extents.z },
		 { half_extents.x, -half_extents.y, -half_extents.z },
		 { half_extents.x, half_extents.y, -half_extents.z },
		 { -half_extents.x, half_extents.y, -half_extents.z } 
	};
	return boxVertices;
}

std::vector<glm::vec3> AYGraphicGenerator::createRectV()
{
	std::vector<glm::vec3> rectVertices;
	rectVertices = {
			{-0.5f, -0.5f, 0.f}, // 左下
			{ 0.5f, -0.5f, 0.f}, // 右下
			{ 0.5f,  0.5f, 0.f}, // 右上
			{-0.5f,  0.5f, 0.f}, // 左上
	};
	return rectVertices;
}

std::vector<uint32_t> AYGraphicGenerator::createCircleI(bool filled, int segments)
{
	std::vector<uint32_t> indices;
	if (filled) {
		for (uint32_t i = 1; i <= segments; ++i) {
			indices.insert(indices.end(), { 0, i, i % segments + 1 });
		}
	}
	else {
		for (uint32_t i = 1; i <= segments; ++i) {
			indices.insert(indices.end(), { i, i % segments + 1 });
		}
	}
	return indices;
}

std::vector<uint32_t> AYGraphicGenerator::createBoxI(bool wireframe)
{
	std::vector<uint32_t> indices;
	if (wireframe)
	{
		indices = {
			0, 1, 1, 2, 2, 3, 3, 0,  // 前面
			4, 5, 5, 6, 6, 7, 7, 4,  // 后面
			0, 4, 1, 5, 2, 6, 3, 7   // 连接前后面的边
		};
	}
	else
	{
		indices = {
			// 前面
			0, 1, 2,
			0, 2, 3,
			// 后面
			4, 5, 6,
			4, 6, 7,
			// 上面
			3, 2, 6,
			3, 6, 7,
			// 下面
			0, 3, 7,
			0, 7, 4,
			// 左面
			0, 1, 5,
			0, 5, 4,
			// 右面
			1, 2, 6,
			1, 6, 5
		};
	}
	return indices;
}

std::vector<uint32_t> AYGraphicGenerator::createRectI(bool filled)
{
	std::vector<uint32_t> indices;
	if (filled) {
		indices = { 0, 1, 2, 0, 2, 3 };
	}
	else {
		indices = { 0, 1, 1, 2, 2, 3, 3, 0 };
	}
	return indices;
}
