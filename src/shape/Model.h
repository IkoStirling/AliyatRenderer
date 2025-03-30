#pragma once
#include "core/core.h"
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>

struct Vertex
{
	float x, y, z;
};

struct TextureCoord
{
	float u, v;
};

struct Normal
{
	float x, y, z;
};

struct Face
{
	std::vector<int> vertexIndices;
	std::vector<int> textureIndices;
	std::vector<int> normalIndices;
};

enum class ModelData_OBJ
{
	VERTEX,
	TEXTURE_COORD,
	NORMAL,
	FACE,
	UNKNOWN
};


extern std::unordered_map<std::string, ModelData_OBJ> objTypeMap;

class Model
{
public:
	static void parseOBJ(const std::string& filePath,
		std::vector<Vertex>& vertices,
		std::vector<TextureCoord>& textureCoords,
		std::vector<Normal>& normals,
		std::vector<Face>& faces
	)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			std::cerr << "Failed to load :" << filePath << std::endl;
			return;
		}

		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string type;
			iss >> type;

			ModelData_OBJ token = _stoe(type);
			switch (token)
			{
			case ModelData_OBJ::VERTEX:
				Vertex vertex;
				iss >> vertex.x >> vertex.y >> vertex.z;
				vertices.push_back(vertex);
				break;
			case ModelData_OBJ::TEXTURE_COORD:
				TextureCoord tex;
				iss >> tex.u >> tex.v;
				textureCoords.push_back(tex);
				break;
			case ModelData_OBJ::NORMAL:
				Normal normal;
				iss >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
				break;
			case ModelData_OBJ::FACE:
			{
				Face face;
				std::string data;
				while (iss >> data)
				{
					std::istringstream viss(data);
					std::string v, vt, vn;
					std::getline(viss, v, '/');
					std::getline(viss, vt, '/');
					std::getline(viss, vn, '/');

					face.vertexIndices.push_back(std::stoi(v) - 1);
					if (!vt.empty())
						face.textureIndices.push_back(std::stoi(vt) - 1);
					if (!vn.empty())
						face.normalIndices.push_back(std::stoi(vn) - 1);
				}
				faces.push_back(face);
			}
				break;
			case ModelData_OBJ::UNKNOWN:
				break;
			}
		}
		file.close();
	}

	static ModelData_OBJ _stoe(const std::string& type)
	{
		auto it = objTypeMap.find(type);
		if (it != objTypeMap.end())
			return it->second;
		return ModelData_OBJ::UNKNOWN;
	}
private:

};