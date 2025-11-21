#pragma once
#include <vector>
#include "AYMathType.h"

struct VertexInfo
{
	AYMath::Vector3 position;
	AYMath::Vector3 normal;
	AYMath::Vector2 uv;
	//AYMath::Vector4 color;    //顶点颜色
};

class AYGraphicGenerator
{
public:
	// -------------VertexInfo----------------
	static std::vector<VertexInfo> createBox(const AYMath::Vector3& half_extents, bool wireframe);


public:
	// ----------------Indices-----------------
	static std::vector<uint32_t> createBoxI(bool wireframe);
	static std::vector<uint32_t> createCircleI(bool filled, int segments);
	static std::vector<uint32_t> createRectI(bool filled);

public:
	// ----------------Vertex------------------
	static std::vector<AYMath::Vector3> createCircleV(float radius, int segments);
	static std::vector<AYMath::Vector3> createBoxV(const AYMath::Vector3& half_extents);
	static std::vector<AYMath::Vector3> createRectV();




public:
	// ----------------Normals-----------------
	static AYMath::Vector3 create2DN();
	static std::vector<AYMath::Vector3> createBoxN();

public:
	// -----------TextureCoordinate------------
	static std::vector<AYMath::Vector2> createRectT();
};