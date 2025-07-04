#include "3DRendering/AYSkyboxRenderer.h"
#include "AYRenderer.h"
#include "AYPath.h"
#include "AYResourceManager.h"
#include <glm/gtc/type_ptr.hpp> ​​
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>

static const float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

//static const float skyboxVertices[] = {
//    // 前面
//    -0.5f, -0.5f,  0.5f,  // 左下
//     0.5f, -0.5f,  0.5f,  // 右下
//     0.5f,  0.5f,  0.5f,  // 右上
//     0.5f,  0.5f,  0.5f,  // 右上
//    -0.5f,  0.5f,  0.5f,  // 左上
//    -0.5f, -0.5f,  0.5f,  // 左下
//
//    // 后面
//    -0.5f, -0.5f, -0.5f,
//     0.5f, -0.5f, -0.5f,
//     0.5f,  0.5f, -0.5f,
//     0.5f,  0.5f, -0.5f,
//    -0.5f,  0.5f, -0.5f,
//    -0.5f, -0.5f, -0.5f,
//
//    // 左面
//    -0.5f,  0.5f,  0.5f,
//    -0.5f,  0.5f, -0.5f,
//    -0.5f, -0.5f, -0.5f,
//    -0.5f, -0.5f, -0.5f,
//    -0.5f, -0.5f,  0.5f,
//    -0.5f,  0.5f,  0.5f,
//
//    // 右面
//     0.5f,  0.5f,  0.5f,
//     0.5f,  0.5f, -0.5f,
//     0.5f, -0.5f, -0.5f,
//     0.5f, -0.5f, -0.5f,
//     0.5f, -0.5f,  0.5f,
//     0.5f,  0.5f,  0.5f,
//
//     // 上面
//     -0.5f,  0.5f, -0.5f,
//      0.5f,  0.5f, -0.5f,
//      0.5f,  0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//     -0.5f,  0.5f,  0.5f,
//     -0.5f,  0.5f, -0.5f,
//
//     // 下面
//     -0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f,  0.5f,
//      0.5f, -0.5f,  0.5f,
//     -0.5f, -0.5f,  0.5f,
//     -0.5f, -0.5f, -0.5f
//};

AYSkyboxRenderer::AYSkyboxRenderer(AYRenderDevice* device, AYRenderer* renderer) :
	_device(device),
	_renderer(renderer),
	_configPath(AYPath::Engine::getPresetConfigPath() + "Renderer/SkyboxRenderer/config.ini")
{
    _setupSkyboxGeometry();
	_loadSkyboxRendererConfigINI();

    loadSkybox({ AYPath::Engine::getPresetTexturePath() + "skyBox.png"},SkyboxType::Equirectangular);
}

AYSkyboxRenderer::~AYSkyboxRenderer()
{
    _saveSkyboxRendererConfigINI();
    if (_skyboxVAO) glDeleteVertexArrays(1, &_skyboxVAO);
    if (_skyboxVBO) glDeleteBuffers(1, &_skyboxVBO);
    if (_cubemapTexture) glDeleteTextures(1, &_cubemapTexture);
    if (_equirectangularTexture) glDeleteTextures(1, &_equirectangularTexture);
}

bool AYSkyboxRenderer::loadSkybox(const std::vector<std::string>& faces, SkyboxType type)
{
    setType(type);

    if (type == SkyboxType::Cube_6Faces && faces.size() == 6) {
        _cubemapTexture = _loadCubemap(faces);
        return _cubemapTexture != 0;
    }
    else if (type == SkyboxType::Equirectangular && faces.size() == 1) {
        _equirectangularTexture = _loadEquirectangularMap(faces[0]);
        return _equirectangularTexture != 0;
    }
    else if (type == SkyboxType::Hemisphere) {
        // 半球天空通常不需要额外纹理
        return true;
    }

    return false;
}

void AYSkyboxRenderer::render(const AYRenderContext& context)
{
    if (_type == SkyboxType::Cube_6Faces && !_cubemapTexture) return;
    if (_type == SkyboxType::Equirectangular && !_equirectangularTexture) return;

    auto shader = _getSkyboxShader();
    if (!shader) return;

    // 保存当前状态
    _device->saveGLState();

    // 设置天空盒渲染状态
    auto stateManager = _device->getGLStateManager();
    stateManager->setDepthTest(false);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    stateManager->bindVertexArray(_skyboxVAO);
    stateManager->useProgram(shader);
    GLint posLoc = glGetAttribLocation(shader, "aPos");
    if (posLoc == -1) {
        std::cerr << "ERROR: Failed to find attribute 'aPos'" << std::endl;
    }

    // 设置uniform
    glm::mat4 view = glm::mat4(glm::mat3(context.currentCamera->getViewMatrix())); // 移除平移部分
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
        1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection;
    if (context.currentCamera->getType() == IAYCamera::Type::ORTHOGRAPHIC_2D) {
        auto viewport = context.currentCamera->getViewport();
        float aspect = viewport.z / viewport.w;
        projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
    }
    else
    {
        projection = context.currentCamera->getProjectionMatrix();
    }
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));

    glUniform1i(glGetUniformLocation(shader, "skyboxType"), static_cast<int>(_type));

    // 绑定纹理
    if (_type == SkyboxType::Cube_6Faces) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemapTexture);
        glUniform1i(glGetUniformLocation(shader, "skybox"), 0);
    }
    else if (_type == SkyboxType::Equirectangular) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _equirectangularTexture);
        glUniform1i(glGetUniformLocation(shader, "equirectangularMap"), 0);
    }

    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, 36);
    // 渲染
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 恢复状态
    _device->restoreGLState();
}

void AYSkyboxRenderer::_setupSkyboxGeometry()
{
    _skyboxVBO = _device->createVertexBuffer(&skyboxVertices, sizeof(skyboxVertices));
    _skyboxVAO = _device->createVertexArray();

    glBindVertexArray(_skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _skyboxVBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

GLuint AYSkyboxRenderer::_loadCubemap(const std::vector<std::string>&faces) 
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < faces.size(); i++) {
        auto tex = AYResourceManager::getInstance().load<AYTexture>(faces[i]);
        if (!tex || !tex->isLoaded()) {
            std::cerr << "Failed to load texture: " << faces[i] << std::endl;
            return 0;
        }
        if (tex) {
            GLenum format = (tex->getChannels() == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, tex->getWidth(), tex->getHeight(),
                0, GL_RGBA, GL_UNSIGNED_BYTE, tex->getPixelData());
        }
        else {
            glDeleteTextures(1, &textureID);
            return 0;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

GLuint AYSkyboxRenderer::_loadEquirectangularMap(const std::string& path) {
    auto tex = AYResourceManager::getInstance().load<AYTexture>(path);
    if (!tex || !tex->isLoaded()) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->getWidth(), tex->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->getPixelData());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

GLuint AYSkyboxRenderer::_getSkyboxShader(bool reload)
{
	return _device->getShaderV(_shaderName, reload, _vertexPath, _fragmentPath);
}

void AYSkyboxRenderer::_loadSkyboxRendererConfigINI()
{
    boost::property_tree::ptree pt;
    if (!std::filesystem::exists(_configPath))
    {
        try
        {
            boost::property_tree::ini_parser::write_ini(_configPath, pt);
        }
        catch (const std::exception& e)
        {
            std::cerr << "INI write failed: " << e.what() << std::endl;
        }
    }

    boost::property_tree::ini_parser::read_ini(_configPath, pt);

    _shaderName = pt.get<std::string>("shader name.skybox", std::string("SkyboxBaseShader"));
    _vertexPath = pt.get<std::string>("shader path.skybox_vertex",
        AYPath::Engine::getPresetShaderPath() + std::string("SkyboxRenderer/skybox.vert"));
    _fragmentPath = pt.get<std::string>("shader path.skybox_fragment",
        AYPath::Engine::getPresetShaderPath() + std::string("SkyboxRenderer/skybox.frag"));
}

void AYSkyboxRenderer::_saveSkyboxRendererConfigINI()
{
    boost::property_tree::ptree pt;

    pt.put("shader name.base", _shaderName);
    pt.put("shader path.base_vertex", _vertexPath);
    pt.put("shader path.base_fragment", _fragmentPath);

    try
    {
        boost::property_tree::ini_parser::write_ini(_configPath, pt);
    }
    catch (const std::exception& e)
    {
        std::cerr << "INI write failed: " << e.what() << std::endl;
    }
}