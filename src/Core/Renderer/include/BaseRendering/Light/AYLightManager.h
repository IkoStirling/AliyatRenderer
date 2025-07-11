#pragma once
#include "AYRenderDevice.h"
#include "STLight.h"
#include <vector>

class AYRenderer;
class AYLightManager {
public:
    AYLightManager(AYRenderDevice* device, AYRenderer* renderer);
    ~AYLightManager();

    // 光源管理接口
    void addDirectionalLight(const STDirectionalLight& light);
    void addPointLight(const STPointLight& light);
    void addSpotLight(const STSpotLight& light);

    void clearAllLights();

    // 更新SSBO数据
    void updateLightData();

    // 绑定SSBO到着色器
    void bindLightData(GLuint bindingPoint = 0);

private:
    AYRenderDevice* _device;
    AYRenderer* _renderer;

    std::vector<STDirectionalLight> _directionalLights;
    std::vector<STPointLight> _pointLights;
    std::vector<STSpotLight> _spotLights;

    // 最大光源数量限制
    static const int MAX_DIRECTIONAL_LIGHTS = 4;
    static const int MAX_POINT_LIGHTS = 16;
    static const int MAX_SPOT_LIGHTS = 8;

#pragma pack(push, 1) // 精确控制内存布局
    struct LightSSBOData {
        // 定向光数据
        alignas(16) glm::vec4 directionalDirections[MAX_DIRECTIONAL_LIGHTS];
        alignas(16) glm::vec4 directionalColors[MAX_DIRECTIONAL_LIGHTS];

        // 点光源数据
        alignas(16) glm::vec4 pointPositions[MAX_POINT_LIGHTS];
        alignas(16) glm::vec4 pointColors[MAX_POINT_LIGHTS];
        alignas(16) glm::vec4 pointParams[MAX_POINT_LIGHTS]; // intensity, radius, constant, linear

        // 聚光灯数据
        alignas(16) glm::vec4 spotPositions[MAX_SPOT_LIGHTS];
        alignas(16) glm::vec4 spotDirections[MAX_SPOT_LIGHTS];
        alignas(16) glm::vec4 spotColors[MAX_SPOT_LIGHTS];
        alignas(16) glm::vec4 spotParams[MAX_SPOT_LIGHTS]; // intensity, cutOff, outerCutOff, 0
        alignas(16) glm::vec4 spotAttenuation[MAX_SPOT_LIGHTS]; // constant, linear, quadratic, 0

        // 光源计数
        alignas(16) glm::ivec4 lightCounts; // x:dir, y:point, z:spot
    };
#pragma pack(pop)

    GLuint _ssbo;
    LightSSBOData _lightData;

    void _setupLightSSBO();
    void _packLightData();
};