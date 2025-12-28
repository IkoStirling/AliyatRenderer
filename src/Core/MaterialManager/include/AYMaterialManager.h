#pragma once
#include "STMaterial.h"
#include "Mod_MaterialManager.h"
#include <unordered_map>
#include <string>

namespace ayt::engine::render
{
    using ayt::engine::resource::Material;

    class AYMaterialManager : public Mod_MaterialManager
    {
    public:
        AYMaterialManager() = default;

        void init() override
        {
            createMaterial({
                .name = "Default",
                .baseColor = glm::vec4(1.f,1.f,1.f,1.f)
                });
        }

        void update(float delta_time) override
        {

        }


        void shutdown() override
        {
            std::unordered_map<std::string, Material> m;
            std::unordered_map<uint32_t, std::string> n;
            _materials.swap(m);
            _nameMap.swap(n);
        }

        void createMaterial(Material mat) {
            std::unique_lock<std::shared_mutex> lock(_materialsMutex);
            uint32_t id = _next++;

            std::string newKey = mat.name;
            if (_materials.find(mat.name) != _materials.end())
            {
                for (int i = 0; ; ++i) {
                    newKey = mat.name + "_" + std::to_string(i);
                    if (_materials.find(newKey) == _materials.end()) {
                        mat.name = newKey;
                        break;
                    }
                }
            }
            mat.id = id;
            _nameMap[id] = newKey;
            _materials[newKey] = mat;
        }

        const Material& getMaterial(uint32_t id) const {
            std::shared_lock<std::shared_mutex> lock(_materialsMutex);
            return _materials.at(_nameMap.at(id));
        }

        const Material& getMaterial(const std::string& name) const {
            std::shared_lock<std::shared_mutex> lock(_materialsMutex);
            return _materials.at(name);
        }

    private:
        std::unordered_map<std::string, Material> _materials;
        std::unordered_map<uint32_t, std::string> _nameMap;
        mutable std::shared_mutex _materialsMutex;
        uint32_t _next = 0;
    };

    REGISTER_MODULE_CLASS("MaterialManager", AYMaterialManager)
}