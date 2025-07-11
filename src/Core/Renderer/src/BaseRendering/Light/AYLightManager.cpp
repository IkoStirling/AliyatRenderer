#include "BaseRendering/Light/AYLightManager.h"
#include "AYRenderer.h"
#include <glm/gtc/type_ptr.hpp> ​​
#include <string>

AYLightManager::AYLightManager(AYRenderDevice* device, AYRenderer* renderer):
    _device(device),
    _renderer(renderer)
{
    _setupLightSSBO();
}

AYLightManager::~AYLightManager()
{
    glDeleteBuffers(1, &_ssbo);
}

void AYLightManager::addDirectionalLight(const STDirectionalLight& light)
{
    if (_directionalLights.size() < MAX_DIRECTIONAL_LIGHTS) {
        _directionalLights.push_back(light);
    }
}

void AYLightManager::addPointLight(const STPointLight& light)
{
    if (_pointLights.size() < MAX_POINT_LIGHTS) {
        _pointLights.push_back(light);
    }
}

void AYLightManager::addSpotLight(const STSpotLight& light)
{
    if (_spotLights.size() < MAX_SPOT_LIGHTS) {
        _spotLights.push_back(light);
    }
}

void AYLightManager::clearAllLights()
{
    _directionalLights.clear();
    _pointLights.clear();
    _spotLights.clear();
}

void AYLightManager::updateLightData()
{
    if (_ssbo == 0) {
        _setupLightSSBO();
        return;
    }
    _packLightData();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo);

    void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        memcpy(ptr, &_lightData, sizeof(LightSSBOData));
        if (!glUnmapBuffer(GL_SHADER_STORAGE_BUFFER)) {
            std::cerr << "Buffer corruption detected!" << std::endl;
        }
    }
    else {
        // 回退到glBufferSubData
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(LightSSBOData), &_lightData);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    AY_CHECK_GL_ERROR("Update lights data failed.");
}

void AYLightManager::bindLightData(GLuint bindingPoint)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, _ssbo);
    AY_CHECK_GL_ERROR("Bind lights data failed.");
}

void AYLightManager::_setupLightSSBO()
{
    if (_ssbo != 0) {
        glDeleteBuffers(1, &_ssbo); // 确保清理旧缓冲区
    }

    glGenBuffers(1, &_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightSSBOData), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    AY_CHECK_GL_ERROR("Setup lights ssbo failed.");
}

void AYLightManager::_packLightData()
{
    memset(&_lightData, 0, sizeof(LightSSBOData));

    // 打包定向光数据
    for (size_t i = 0; i < _directionalLights.size() && i < MAX_DIRECTIONAL_LIGHTS; ++i) {
        const auto& light = _directionalLights[i];
        _lightData.directionalDirections[i] = glm::vec4(light.direction, 0.0f);
        _lightData.directionalColors[i] = glm::vec4(light.color, light.intensity);
    }

    // 打包点光源数据
    for (size_t i = 0; i < _pointLights.size() && i < MAX_POINT_LIGHTS; ++i) {
        const auto& light = _pointLights[i];
        _lightData.pointPositions[i] = glm::vec4(light.position, 1.0f);
        _lightData.pointColors[i] = glm::vec4(light.color, light.quadratic);
        _lightData.pointParams[i] = glm::vec4(
            light.intensity,
            light.radius,
            light.constant,
            light.linear
        );
    }

    // 打包聚光灯数据
    for (size_t i = 0; i < _spotLights.size() && i < MAX_SPOT_LIGHTS; ++i) {
        const auto& light = _spotLights[i];
        _lightData.spotPositions[i] = glm::vec4(light.position, 1.0f);
        _lightData.spotDirections[i] = glm::vec4(light.direction, 0.0f);
        _lightData.spotColors[i] = glm::vec4(light.color, 0.0f);
        _lightData.spotParams[i] = glm::vec4(
            light.intensity,
            light.cutOff,
            light.outerCutOff,
            0.0f
        );
        _lightData.spotAttenuation[i] = glm::vec4(
            light.constant,
            light.linear,
            light.quadratic,
            0.0f
        );
    }

    // 设置光源计数
    _lightData.lightCounts = glm::ivec4(
        _directionalLights.size(),
        _pointLights.size(),
        _spotLights.size(),
        0
    );
}
