#include "Model.h"


std::unordered_map<std::string, ModelData_OBJ> objTypeMap =
{
	{"v", ModelData_OBJ::VERTEX},
	{"vt", ModelData_OBJ::TEXTURE_COORD},
	{"vn", ModelData_OBJ::NORMAL},
	{"f", ModelData_OBJ::FACE}
};