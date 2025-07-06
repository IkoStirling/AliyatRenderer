#pragma once
#include "AYRenderDevice.h"
#include "BaseRendering/AYRenderContext.h"
#include "AYConfigWrapper.h"
#include <vector>
#include <string>

class AYRenderer;
class AYRenderContext;

class AYSkyboxRenderer {
public:
    AYSkyboxRenderer(AYRenderDevice* device, AYRenderer* renderer);
    ~AYSkyboxRenderer();
public:
    enum class SkyboxType {
        Cube_6Faces,        // ��ͳ6����������ͼ
        Equirectangular,    // ����ȫ����ͼ
        Hemisphere          // �������
    };

    void render(const AYRenderContext& context);

    bool loadSkybox(const std::vector<std::string>& faces, SkyboxType type = SkyboxType::Cube_6Faces);

    void setType(SkyboxType type) { _type = type; }
    SkyboxType getType() const { return _type; }
private:
    AYRenderDevice* _device;
    AYRenderer* _renderer;
    SkyboxType _type;

    GLuint _cubemapTexture = 0;
    GLuint _equirectangularTexture = 0;
    GLuint _skyboxVAO = 0, _skyboxVBO = 0;

    AYConfigWrapper _config;
    const std::string _configPath;
    std::string _shaderName;
    std::string _vertexPath;
    std::string _fragmentPath;

    void _loadSkyboxRendererConfigINI();
    void _saveSkyboxRendererConfigINI();
    void _setupSkyboxGeometry();
    GLuint _loadCubemap(const std::vector<std::string>& faces);
    GLuint _loadEquirectangularMap(const std::string& path);
    GLuint _getSkyboxShader(bool reload = false);
};