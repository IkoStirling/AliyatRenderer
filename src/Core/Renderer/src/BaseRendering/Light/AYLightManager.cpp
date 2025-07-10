#pragma once
#include "Light.h"
#include <vector>

class LightManager {
public:
    static LightManager& Get() {
        static LightManager instance;
        return instance;
    }

    // 光源管理接口
    void AddDirectionalLight(const DirectionalLight& light);
    void AddPointLight(const PointLight& light);
    void AddSpotLight(const SpotLight& light);

    void ClearAllLights();

    // 更新Shader中的光照数据
    void UploadLightData(GLuint shaderProgram);

private:
    LightManager() = default;

    std::vector<DirectionalLight> m_DirectionalLights;
    std::vector<PointLight> m_PointLights;
    std::vector<SpotLight> m_SpotLights;

    // 最大光源数量限制
    static const int MAX_DIRECTIONAL_LIGHTS = 4;
    static const int MAX_POINT_LIGHTS = 16;
    static const int MAX_SPOT_LIGHTS = 8;
};