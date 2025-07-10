#pragma once
#include "STMaterial.h"
#include <unordered_map>

class AYMaterialManager 
{
public:
    AYMaterialManager()
    {
        createMaterial({.baseColor = glm::vec4(1.f,1.f,1.f,1.f)});
    }
    uint32_t createMaterial(const STMaterial& mat) {
        uint32_t id = _next++;
        _materials[id] = mat;
        return id;
    }

    const STMaterial& getMaterial(uint32_t id) const {
        return _materials.at(id);
    }

private:
    std::unordered_map<uint32_t, STMaterial> _materials;
    uint32_t _next = 0;
};