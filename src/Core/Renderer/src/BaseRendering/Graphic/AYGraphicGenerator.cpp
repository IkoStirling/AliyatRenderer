#include "BaseRendering/Graphic/AYGraphicGenerator.h"
#define M_PI 3.141592659f

std::vector<VertexInfo> AYGraphicGenerator::createBox(const AYMath::Vector3& half_extents, bool wireframe)
{
	std::vector<VertexInfo> vertices;

	// 定义立方体6个面的法线方向
	const AYMath::Vector3 normals[6] = {
		{ 0.0f,  0.0f,  0.5f},  // 前面 (Z+)
		{ 0.0f,  0.0f, -0.5f},  // 后面 (Z-)
		{ 0.0f,  0.5f,  0.0f},  // 上面 (Y+)
		{ 0.0f, -0.5f,  0.0f},  // 下面 (Y-)
		{-0.5f,  0.0f,  0.0f},  // 左面 (X-)
		{ 0.5f,  0.0f,  0.0f}   // 右面 (X+)
	};

	// 定义立方体6个面的纹理坐标（每个面4个顶点）
	const AYMath::Vector2 uvCoords[4] = {
		{0.0f, 0.0f},  // 左下
		{1.0f, 0.0f},  // 右下
		{1.0f, 1.0f},  // 右上
		{0.0f, 1.0f}   // 左上
	};

	// 为每个面生成4个顶点（共24个顶点，适合硬表面渲染）
	for (int face = 0; face < 6; ++face) {
		AYMath::Vector3 normal = normals[face];

		// 根据面类型确定顶点位置
		switch (face) {
		case 0: // 前面 (Z+)
			vertices.push_back({ {-half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[3] });
			break;

		case 1: // 后面 (Z-)
			vertices.push_back({ { half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ {-half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ {-half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ { half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[3] });
			break;

		case 2: // 上面 (Y+)
			vertices.push_back({ {-half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[3] });
			break;

		case 3: // 下面 (Y-)
			vertices.push_back({ {-half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[3] });
			break;

		case 4: // 左面 (X-)
			vertices.push_back({ {-half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ {-half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ {-half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[3] });
			break;

		case 5: // 右面 (X+)
			vertices.push_back({ { half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ { half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[3] });
			break;
		}
	}

	return vertices;
}

std::vector<AYMath::Vector3> AYGraphicGenerator::createCircleV(float radius, int segments)
{
	std::vector<AYMath::Vector3> circleVertices;
	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		circleVertices.push_back(AYMath::Vector3(
			cos(angle) * radius,
			sin(angle) * radius,
			0.0f
		));
	}
	return circleVertices;
}

std::vector<AYMath::Vector3> AYGraphicGenerator::createBoxV(const AYMath::Vector3& half_extents)
{
	std::vector<AYMath::Vector3> boxVertices;
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

std::vector<AYMath::Vector3> AYGraphicGenerator::createRectV()
{
	std::vector<AYMath::Vector3> rectVertices;
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
	if (wireframe) 
	{
		// 线框模式索引（使用8个顶点）
		return {
			0,1,1,2,2,3,3,0,    // 前面
			4,5,5,6,6,7,7,4,    // 后面
			0,4,1,5,2,6,3,7     // 连接线
		};
	}
	else 
	{
		// 实体模式索引（使用24个顶点，每面4个）
		std::vector<uint32_t> indices;
		for (uint32_t face = 0; face < 6; ++face) {
			uint32_t baseIndex = face * 4;
			// 每个面两个三角形
			indices.push_back(baseIndex);
			indices.push_back(baseIndex + 1);
			indices.push_back(baseIndex + 2);

			indices.push_back(baseIndex);
			indices.push_back(baseIndex + 2);
			indices.push_back(baseIndex + 3);
		}
		return indices;
	}
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

AYMath::Vector3 AYGraphicGenerator::create2DN()
{
	return AYMath::Vector3(0.0f, 0.0f, 1.0f);
}

std::vector<AYMath::Vector3> AYGraphicGenerator::createBoxN()
{
	std::vector<AYMath::Vector3> normals;
	normals = {
		AYMath::Vector3(1, 0, 0),   // +X
		AYMath::Vector3(-1, 0, 0),  // -X
		AYMath::Vector3(0, 1, 0),   // +Y
		AYMath::Vector3(0, -1, 0),  // -Y
		AYMath::Vector3(0, 0, 1),   // +Z
		AYMath::Vector3(0, 0, -1)   // -Z
	};
	return normals;
}

std::vector<AYMath::Vector2> AYGraphicGenerator::createRectT()
{
	return {
		{0.0f, 0.0f},  // 左下
		{1.0f, 0.0f},  // 右下
		{1.0f, 1.0f},  // 右上
		{0.0f, 1.0f}   // 左上
	};
}
