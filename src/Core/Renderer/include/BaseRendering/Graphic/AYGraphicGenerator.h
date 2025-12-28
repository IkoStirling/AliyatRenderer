#pragma once
#include <vector>
#include "AYMathType.h"
namespace ayt::engine::render
{
	struct VertexInfo
	{
		math::Vector3 position;
		math::Vector3 normal;
		math::Vector2 uv;
		//math::Vector4 color;    //顶点颜色
	};

	class GraphicGenerator
	{
	public:
		// -------------VertexInfo----------------
		static std::vector<VertexInfo> createBox(const math::Vector3& half_extents, bool wireframe);


	public:
		// ----------------Indices-----------------
		static std::vector<uint32_t> createBoxI(bool wireframe);
		static std::vector<uint32_t> createCircleI(bool filled, int segments);
		static std::vector<uint32_t> createRectI(bool filled);

	public:
		// ----------------Vertex------------------
		static std::vector<math::Vector3> createCircleV(float radius, int segments);
		static std::vector<math::Vector3> createBoxV(const math::Vector3& half_extents);
		static std::vector<math::Vector3> createRectV();




	public:
		// ----------------Normals-----------------
		static math::Vector3 create2DN();
		static std::vector<math::Vector3> createBoxN();

	public:
		// -----------TextureCoordinate------------
		static std::vector<math::Vector2> createRectT();
	};
}