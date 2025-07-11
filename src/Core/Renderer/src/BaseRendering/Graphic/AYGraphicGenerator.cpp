#include "BaseRendering/Graphic/AYGraphicGenerator.h"
#define M_PI 3.141592659f

std::vector<VertexInfo> AYGraphicGenerator::createBox(const glm::vec3& half_extents, bool wireframe)
{
	std::vector<VertexInfo> vertices;

	// ����������6����ķ��߷���
	const glm::vec3 normals[6] = {
		{ 0.0f,  0.0f,  1.0f},  // ǰ�� (Z+)
		{ 0.0f,  0.0f, -1.0f},  // ���� (Z-)
		{ 0.0f,  1.0f,  0.0f},  // ���� (Y+)
		{ 0.0f, -1.0f,  0.0f},  // ���� (Y-)
		{-1.0f,  0.0f,  0.0f},  // ���� (X-)
		{ 1.0f,  0.0f,  0.0f}   // ���� (X+)
	};

	// ����������6������������꣨ÿ����4�����㣩
	const glm::vec2 uvCoords[4] = {
		{0.0f, 0.0f},  // ����
		{1.0f, 0.0f},  // ����
		{1.0f, 1.0f},  // ����
		{0.0f, 1.0f}   // ����
	};

	// Ϊÿ��������4�����㣨��24�����㣬�ʺ�Ӳ������Ⱦ��
	for (int face = 0; face < 6; ++face) {
		glm::vec3 normal = normals[face];

		// ����������ȷ������λ��
		switch (face) {
		case 0: // ǰ�� (Z+)
			vertices.push_back({ {-half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[3] });
			break;

		case 1: // ���� (Z-)
			vertices.push_back({ { half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ {-half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ {-half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ { half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[3] });
			break;

		case 2: // ���� (Y+)
			vertices.push_back({ {-half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[3] });
			break;

		case 3: // ���� (Y-)
			vertices.push_back({ {-half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[3] });
			break;

		case 4: // ���� (X-)
			vertices.push_back({ {-half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ {-half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ {-half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ {-half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[3] });
			break;

		case 5: // ���� (X+)
			vertices.push_back({ { half_extents.x, -half_extents.y,  half_extents.z}, normal, uvCoords[0] });
			vertices.push_back({ { half_extents.x, -half_extents.y, -half_extents.z}, normal, uvCoords[1] });
			vertices.push_back({ { half_extents.x,  half_extents.y, -half_extents.z}, normal, uvCoords[2] });
			vertices.push_back({ { half_extents.x,  half_extents.y,  half_extents.z}, normal, uvCoords[3] });
			break;
		}
	}

	return vertices;
}

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

		 // ���棨z=-half_extents.z��
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
			{-0.5f, -0.5f, 0.f}, // ����
			{ 0.5f, -0.5f, 0.f}, // ����
			{ 0.5f,  0.5f, 0.f}, // ����
			{-0.5f,  0.5f, 0.f}, // ����
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
		// �߿�ģʽ������ʹ��8�����㣩
		return {
			0,1,1,2,2,3,3,0,    // ǰ��
			4,5,5,6,6,7,7,4,    // ����
			0,4,1,5,2,6,3,7     // ������
		};
	}
	else 
	{
		// ʵ��ģʽ������ʹ��24�����㣬ÿ��4����
		std::vector<uint32_t> indices;
		for (uint32_t face = 0; face < 6; ++face) {
			uint32_t baseIndex = face * 4;
			// ÿ��������������
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

glm::vec3 AYGraphicGenerator::create2DN()
{
	return glm::vec3(0.0f, 0.0f, 1.0f);
}

std::vector<glm::vec3> AYGraphicGenerator::createBoxN()
{
	std::vector<glm::vec3> normals;
	normals = {
		glm::vec3(1, 0, 0),   // +X
		glm::vec3(-1, 0, 0),  // -X
		glm::vec3(0, 1, 0),   // +Y
		glm::vec3(0, -1, 0),  // -Y
		glm::vec3(0, 0, 1),   // +Z
		glm::vec3(0, 0, -1)   // -Z
	};
	return normals;
}

std::vector<glm::vec2> AYGraphicGenerator::createBoxV()
{
	std::vector<glm::vec2> texCoord;
	texCoord = {
		{0.0f, 0.0f},  // ����
		{1.0f, 0.0f},  // ����
		{1.0f, 1.0f},  // ����
		{0.0f, 1.0f}   // ����
	};
	return texCoord;
}
