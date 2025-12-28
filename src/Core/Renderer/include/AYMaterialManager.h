#pragma once
#include "STMaterial.h"
#include "Mod_MaterialManager.h"
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <memory>
#include <stdexcept> // 用于抛出异常

namespace ayt::engine::render
{
    using ayt::engine::resource::Material;

    class MaterialManager : public Mod_MaterialManager
    {
    public:
        // 最大重试次数：避免命名冲突时无限循环
        static constexpr int MAX_RETRY_COUNT = 1000;

        MaterialManager() = default;

        void init() override
        {
            // 使用右值引用重载，消除拷贝
            createMaterial(Material{
                .name = "Default",
                .baseColor = glm::vec4(1.f,1.f,1.f,1.f)
                });
        }

        void update(float delta_time) override
        {
        }

        void shutdown() override
        {
            // 用空map交换，确保原有资源被销毁（兼容原有逻辑，优化可读性）
            std::unordered_map<std::string, Material> materialTemp;
            std::unordered_map<uint32_t, std::string> nameMapTemp;
            {
                // 加独占锁，确保交换时线程安全
                std::unique_lock<std::shared_mutex> lock(_materialsMutex);
                _materials.swap(materialTemp);
                _nameMap.swap(nameMapTemp);
                _next = 0; // 重置计数器
            }
        }

        uint32_t createMaterial(Material&& mat) {
            return createMaterialImpl(std::move(mat));
        }

        uint32_t createMaterial(const Material& mat) {
            return createMaterialImpl(mat);
        }

        const Material& getMaterial(uint32_t id) const override {
            std::shared_lock<std::shared_mutex> lock(_materialsMutex);
            auto nameIt = _nameMap.find(id);
            if (nameIt == _nameMap.end()) {
                throw std::out_of_range("Material id not found: " + std::to_string(id));
            }
            auto matIt = _materials.find(nameIt->second);
            if (matIt == _materials.end()) {
                throw std::logic_error("Material name mismatch for id: " + std::to_string(id));
            }
            return matIt->second;
        }

        const Material& getMaterial(const std::string& name) const override {
            std::shared_lock<std::shared_mutex> lock(_materialsMutex);
            auto matIt = _materials.find(name);
            if (matIt == _materials.end()) {
                throw std::out_of_range("Material name not found: " + name);
            }
            return matIt->second;
        }

    private:
        // 核心实现：抽取公共逻辑，避免代码冗余
        template<typename MatType>
        uint32_t createMaterialImpl(MatType&& mat) {
            std::unique_lock<std::shared_mutex> lock(_materialsMutex);

            std::string matName = mat.name;
            std::string newKey = matName;
            int retryCount = 0;

            // 命名冲突处理：带最大重试次数，避免无限循环
            while (_materials.find(newKey) != _materials.end()) {
                if (retryCount >= MAX_RETRY_COUNT) {
                    throw std::runtime_error("Failed to create material: max retry count reached for name: " + matName);
                }
                newKey = matName + "_" + std::to_string(retryCount);
                retryCount++;
            }

            // 分配id（先分配，再插入，逻辑更清晰）
            uint32_t newId = _next++;
            // 修改局部副本，不影响外部传入的对象
            Material newMat = std::forward<MatType>(mat);
            newMat.name = newKey;
            newMat.id = newId;

            // 使用try_emplace：仅当key不存在时插入，提升异常安全性
            // 避免operator[]的默认构造+赋值带来的冗余和异常风险
            auto [matIt, matInserted] = _materials.try_emplace(newKey, std::move(newMat));
            auto [nameIt, nameInserted] = _nameMap.try_emplace(newId, newKey);

            // 插入失败时回滚id计数器（理论上不会失败，因已提前检查key不存在）
            if (!matInserted || !nameInserted) {
                _next--;
                throw std::runtime_error("Failed to insert material: " + newKey);
            }

            return newId;
        }

        std::unordered_map<std::string, Material> _materials;
        std::unordered_map<uint32_t, std::string> _nameMap;
        mutable std::shared_mutex _materialsMutex;
        uint32_t _next = 0;
    };

    REGISTER_MODULE_CLASS("MaterialManager", MaterialManager)
}