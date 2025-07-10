#pragma once
#include "Light.h"
#include <vector>

class LightManager {
public:
    static LightManager& Get() {
        static LightManager instance;
        return instance;
    }

    // ��Դ����ӿ�
    void AddDirectionalLight(const DirectionalLight& light);
    void AddPointLight(const PointLight& light);
    void AddSpotLight(const SpotLight& light);

    void ClearAllLights();

    // ����Shader�еĹ�������
    void UploadLightData(GLuint shaderProgram);

private:
    LightManager() = default;

    std::vector<DirectionalLight> m_DirectionalLights;
    std::vector<PointLight> m_PointLights;
    std::vector<SpotLight> m_SpotLights;

    // ����Դ��������
    static const int MAX_DIRECTIONAL_LIGHTS = 4;
    static const int MAX_POINT_LIGHTS = 16;
    static const int MAX_SPOT_LIGHTS = 8;
};