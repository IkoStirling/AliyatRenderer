#pragma once
#include "IAYResource.h"
#include "AYResourceRegistry.h"
#include "AYMemoryPool.h"
#include "STMaterial.h"
#include "AYMaterialManager.h"
#include "STMesh.h"
#include <filesystem>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
namespace ayt::engine::resource
{
    class AYModel : public IAYResource
    {
        //SUPPORT_MEMORY_POOL(AYModel)
        DECLARE_RESOURCE_CLASS(AYModel, "models")
    public:
        AYModel() = default;
        virtual ~AYModel() {
            releaseData();
        }

        // 获取模型数据
        const std::vector<Mesh>& getMeshes() const { return _meshes; }
        const std::vector<std::string> getMaterials() const {
            std::vector<std::string> result;
            for (auto& mesh : _meshes)
            {
                result.push_back(mesh.materialName);
            }
            return result;
        }

        virtual bool load(const std::string& filepath) override {
            if (_loaded) return true;

            _filepath = filepath;

            // 使用Assimp加载模型
            Assimp::Importer importer;
            const aiScene* _assimpScene = importer.ReadFile(filepath,
                aiProcess_Triangulate |         // 导入图元转化为三角形
                //aiProcess_GenNormals |          // 生成法线
                //aiProcess_CalcTangentSpace |    // 计算切线空间（用于法线贴图）
                //aiProcess_FlipUVs             // 翻转UV
                aiProcess_OptimizeMeshes |            // 优化网格（合并重复数据）
                aiProcess_OptimizeGraph |             // 优化节点树结构
                aiProcess_RemoveRedundantMaterials |  // 移除无用材质
                aiProcess_JoinIdenticalVertices       // 合并相同顶点
            );

            if (!_assimpScene || _assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !_assimpScene->mRootNode) {
                std::cerr << "[AYModel] Assimp load failed: " << importer.GetErrorString() << std::endl;
                return false;
            }

            std::filesystem::path fp(filepath);
            std::string prefix = fp.stem().string() + "_";

            // 一个mesh代表一组渲染数据，和一个材质关联，一个模型可能包含多个网格数据
            // 这里分别将网格数据和材质数据提取，通过 材质名称 映射
            // 处理网格数据
            _meshes.resize(_assimpScene->mNumMeshes);
            for (unsigned int i = 0; i < _assimpScene->mNumMeshes; i++) {
                const aiMesh* mesh = _assimpScene->mMeshes[i];
                processMesh(mesh, _meshes[i], prefix);
                // 关联材质
                if (mesh->mMaterialIndex >= 0) {
                    _meshes[i].materialName = prefix + _assimpScene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
                }
            }

            calculateModelBoundingBox();

            // 处理材质数据
            for (unsigned int i = 0; i < _assimpScene->mNumMaterials; i++) {
                const aiMaterial* material = _assimpScene->mMaterials[i];
                processMaterial(material, prefix);
            }



            IAYResource::load(filepath);
            _loaded = true;
            return true;
        }

        virtual bool unload() override {
            if (!_loaded) return true;
            std::lock_guard<std::mutex> lock(_dataMutex);
            releaseData();
            _loaded = false;
            return true;
        }

        virtual size_t sizeInBytes() override {
            size_t total = 0;
            for (const auto& mesh : _meshes) {
                total += mesh.vertices.size() * sizeof(glm::vec3);
                total += mesh.normals.size() * sizeof(glm::vec3);
                total += mesh.texCoords.size() * sizeof(glm::vec2);
                total += mesh.colors.size() * sizeof(glm::vec4);
                total += mesh.indices.size() * sizeof(unsigned int);
            }
            return total;
        }

    private:
        static glm::mat4 convertAiMatrixToGlm(const aiMatrix4x4& from) {
            glm::mat4 to;
            to[0][0] = from.a1; to[0][1] = from.b1; to[0][2] = from.c1; to[0][3] = from.d1;
            to[1][0] = from.a2; to[1][1] = from.b2; to[1][2] = from.c2; to[1][3] = from.d2;
            to[2][0] = from.a3; to[2][1] = from.b3; to[2][2] = from.c3; to[2][3] = from.d3;
            to[3][0] = from.a4; to[3][1] = from.b4; to[3][2] = from.c4; to[3][3] = from.d4;
            return to;
        }

        void processMesh(const aiMesh* aiMesh, Mesh& mesh, const std::string& prefix) {
            // 处理顶点数据
            mesh.vertices.resize(aiMesh->mNumVertices);
            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                mesh.vertices[i] = glm::vec3(
                    aiMesh->mVertices[i].x,
                    aiMesh->mVertices[i].y,
                    aiMesh->mVertices[i].z);
            }

            // 处理法线数据
            if (aiMesh->HasNormals()) {
                mesh.normals.resize(aiMesh->mNumVertices);
                for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                    mesh.normals[i] = glm::vec3(
                        aiMesh->mNormals[i].x,
                        aiMesh->mNormals[i].y,
                        aiMesh->mNormals[i].z);
                }
            }

            // 处理纹理坐标
            if (aiMesh->HasTextureCoords(0)) {
                mesh.texCoords.resize(aiMesh->mNumVertices);
                for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                    mesh.texCoords[i] = glm::vec2(
                        aiMesh->mTextureCoords[0][i].x,
                        aiMesh->mTextureCoords[0][i].y);
                }
            }

            // 处理顶点颜色
            if (aiMesh->HasVertexColors(0)) {
                mesh.colors.resize(aiMesh->mNumVertices);
                for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                    mesh.colors[i] = glm::vec4(
                        aiMesh->mColors[0][i].r,
                        aiMesh->mColors[0][i].g,
                        aiMesh->mColors[0][i].b,
                        aiMesh->mColors[0][i].a);
                }
            }

            if (aiMesh->HasBones())
            {
                // 用于将骨骼名称映射到索引
                std::unordered_map<std::string, uint32_t> boneNameToIndex;
                std::vector<glm::mat4> boneOffsetMatrices; // 每个骨骼的 offset matrix

                // 首先遍历所有骨骼，建立索引并收集 offset 矩阵
                for (unsigned int b = 0; b < aiMesh->mNumBones; b++)
                {
                    const aiBone* aiBone = aiMesh->mBones[b];
                    std::string boneName = aiBone->mName.C_Str();

                    // 如果该骨骼尚未记录，则添加
                    if (boneNameToIndex.find(boneName) == boneNameToIndex.end())
                    {
                        uint32_t boneIndex = static_cast<uint32_t>(boneNameToIndex.size());
                        boneNameToIndex[boneName] = boneIndex;
                        boneOffsetMatrices.push_back(convertAiMatrixToGlm(aiBone->mOffsetMatrix));
                    }
                }

                // 初始化每个顶点的骨骼绑定数据
                const size_t numVertices = aiMesh->mNumVertices;
                mesh.boneIDs.resize(numVertices, glm::ivec4(-1));         // 默认无绑定
                mesh.boneWeights.resize(numVertices, glm::vec4(0.0f));    // 默认权重为0

                mesh.boneOffsetMatrices = boneOffsetMatrices; // 保存偏移矩阵（可选，但推荐）

                // 遍历每个骨骼，为每个顶点分配权重
                for (unsigned int b = 0; b < aiMesh->mNumBones; b++)
                {
                    const aiBone* aiBone = aiMesh->mBones[b];
                    std::string boneName = aiBone->mName.C_Str();
                    uint32_t boneIndex = boneNameToIndex[boneName]; // 我们自己分配的索引

                    // 遍历该骨骼的所有权重
                    for (unsigned int w = 0; w < aiBone->mNumWeights; w++)
                    {
                        const aiVertexWeight& weight = aiBone->mWeights[w];
                        uint32_t vertexId = weight.mVertexId;

                        // 当前顶点的权重数据
                        glm::ivec4& boneID = mesh.boneIDs[vertexId];
                        glm::vec4& boneWeight = mesh.boneWeights[vertexId];

                        // 找到一个尚未使用的权重槽（值为0）
                        for (int i = 0; i < 4; i++)
                        {
                            if (boneWeight[i] == 0.0f)
                            {
                                boneID[i] = static_cast<int32_t>(boneIndex);
                                boneWeight[i] = weight.mWeight;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                // 没有骨骼，初始化为空
                const size_t numVertices = aiMesh->mNumVertices;
                mesh.boneIDs.resize(numVertices, glm::ivec4(-1));
                mesh.boneWeights.resize(numVertices, glm::vec4(0.0f));
                mesh.boneOffsetMatrices.clear(); // 无骨骼，无需偏移矩阵
            }

            // 处理索引数据
            mesh.indices.reserve(aiMesh->mNumFaces * 3);
            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
                const aiFace& face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    mesh.indices.push_back(face.mIndices[j]);
                }
            }

            // 计算包围盒
            mesh.calculateBoundingBox();


        }

        void processMaterial(const aiMaterial* aiMaterial, const std::string& prefix) {
            aiString name;
            aiMaterial->Get(AI_MATKEY_NAME, name);
            Material material;
            material.name = prefix + name.C_Str();

            // ======================
            // 1. 基础颜色 (Base Color / Albedo)
            // ======================
            aiColor4D baseColor;
            if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS) {
                material.baseColor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
            }
            bool hasTransparency = (material.baseColor.a < 0.99f);

            // ======================
            // 2. PBR 属性
            // ======================
            float value;

            if (aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, value) == AI_SUCCESS) {
                material.metallic = value;
            }

            if (aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, value) == AI_SUCCESS) {
                material.roughness = value;
            }

            if (aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, baseColor) == AI_SUCCESS) {
                material.ao = baseColor.r;  // 有些模型将 AO 存在 Ambient 颜色中
            }

            float opacity;
            if (aiMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
                material.baseColor.a *= opacity; // 合并到基础颜色的Alpha
            }

            aiColor3D emissiveColor;
            if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS) {
                material.emissiveColor = glm::vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
            }

            // 标记双面渲染
            int twoSided;
            if (aiMaterial->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
                material.isTwoSided = (twoSided != 0);
            }

            // ======================
            // 3. 纹理路径
            // ======================

            aiString path;

            std::filesystem::path modelDir = std::filesystem::path(_filepath).parent_path();
            // 辅助函数：将相对路径转换为绝对路径
            auto resolveTexturePath = [this, &modelDir](const std::string& texPath) -> std::string {
                if (texPath.empty()) return "";

                std::filesystem::path fullPath = modelDir / texPath;
                return fullPath.string();
                };

            // [1] 漫反射贴图 → albedoTexture
            if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
                material.albedoTexture = resolveTexturePath(path.C_Str());
            }

            // 检查透明度纹理
            if (aiMaterial->GetTexture(aiTextureType_OPACITY, 0, &path) == AI_SUCCESS) {
                std::string tmp(resolveTexturePath(path.C_Str()));
                if (tmp != material.albedoTexture)
                {
                    material.opacityTexture = tmp;
                    hasTransparency = true;
                }
            }

            // [2] 法线贴图 → normalTexture
            if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS) {
                material.normalTexture = resolveTexturePath(path.C_Str());
            }
            // 或者尝试 aiTextureType_HEIGHT（有些导出器把法线放在 HEIGHT）
            else if (aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS) {
                material.normalTexture = resolveTexturePath(path.C_Str());
            }

            // [3] 金属度贴图 → metallicTexture
            if (aiMaterial->GetTexture(aiTextureType_METALNESS, 0, &path) == AI_SUCCESS) {
                material.metallicTexture = resolveTexturePath(path.C_Str());
            }

            // [4] 粗糙度贴图 → roughnessTexture
            if (aiMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path) == AI_SUCCESS) {
                material.roughnessTexture = resolveTexturePath(path.C_Str());
            }

            // [5] AO 贴图 → aoTexture
            if (aiMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &path) == AI_SUCCESS) {
                material.aoTexture = resolveTexturePath(path.C_Str());
            }

            // [6] 自发光贴图 → emissiveTexture
            if (aiMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS) {
                material.emissiveTexture = resolveTexturePath(path.C_Str());
            }

            material.type = hasTransparency ? Material::Type::Transparent : Material::Type::Opaque;

            auto materialManager = GET_CAST_MODULE(render::AYMaterialManager, "MaterialManager");
            materialManager->createMaterial(material);
        }

        void releaseData() {
            _meshes.clear();
        }

        void calculateModelBoundingBox() {
            _modelBoundingBoxMin = glm::vec3(FLT_MAX);
            _modelBoundingBoxMax = glm::vec3(-FLT_MAX);
            for (const auto& mesh : _meshes) {
                _modelBoundingBoxMin = glm::min(_modelBoundingBoxMin, mesh.boundingBoxMin);
                _modelBoundingBoxMax = glm::max(_modelBoundingBoxMax, mesh.boundingBoxMax);
            }
        }

        std::vector<Mesh> _meshes;
        std::string _filepath;
        mutable std::mutex _dataMutex;
        glm::vec3 _modelBoundingBoxMin;
        glm::vec3 _modelBoundingBoxMax;


    };

    REGISTER_RESOURCE_CLASS(AYModel, 0)
}