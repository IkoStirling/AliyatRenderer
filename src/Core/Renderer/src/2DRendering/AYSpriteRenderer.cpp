#include "2DRendering/AYSpriteRenderer.h"
#include "AYRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> ​​

const char* SPRITE_VERTEX_SHADER = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    out vec2 TexCoord;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform float flipH;
    uniform float flipW;
    
    void main() {
        vec2 texCoord = aTexCoord;
        if(flipH < 0.0) {
            texCoord.x = 1.0 - texCoord.x; // 水平翻转UV坐标
        }
        if(flipW < 0.0) {
            texCoord.y = 1.0 - texCoord.y; // 垂直翻转UV坐标
        }
        TexCoord = texCoord;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

const char* SPRITE_FRAGMENT_SHADER = R"(
    #version 460 core
    in vec2 TexCoord;
    out vec4 FragColor;
    
    uniform sampler2D texture1;
    uniform vec4 spriteColor;
    
    void main() {
        FragColor = texture(texture1, TexCoord) * spriteColor;
        if (FragColor.a < 0.1)
            discard;
    }
)";

const char* SPRITE_VERTEX_SHADER_ATLAS = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform vec2 uvOffset;
        uniform vec2 uvSize;
        uniform float flipH;
        uniform float flipW;
        
        void main() {
            vec2 texCoord = aTexCoord;
            if(flipH < 0.0) texCoord.x = 1.0 - texCoord.x; 
            if(flipW < 0.0) texCoord.y = 1.0 - texCoord.y; 

            TexCoord = uvOffset + texCoord * uvSize;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

AYSpriteRenderer::AYSpriteRenderer(AYRenderDevice* device, AYRenderer* renderer):
    _device(device),
    _renderer(renderer),
    _configPath("@config/Renderer/SpriteRenderer/config.ini")
{
    _loadSpriteRendererConfigINI();
    _initBuffers();
}

AYSpriteRenderer::~AYSpriteRenderer()
{

}

void AYSpriteRenderer::shutdown()
{
    _saveSpriteRendererConfigINI();
    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
    }
    if (_vbo) {
        glDeleteBuffers(1, &_vbo);
    }
}


void AYSpriteRenderer::drawSprite(GLuint texture,
    const STTransform& transform,
    const glm::vec2& size,
    const glm::vec4& color,
    bool flipHorizontal,
    bool flipVertical,
    const glm::vec2& origin
)
{
    drawSprite3D(
        texture,
        transform,
        glm::vec3(size, 1.f),
        color,
        flipHorizontal,
        flipVertical,
        glm::vec3(origin, 0.f)
    );
}

void AYSpriteRenderer::drawSpriteFromAtlas(GLuint texture,
    const STTransform& transform,
    const glm::vec2& size,
    const glm::vec2& uvOffset,
    const glm::vec2& uvSize, 
    const glm::vec4& color,
    bool flipHorizontal,
    bool flipVertical,
    const glm::vec2& origin
)
{
    drawSpriteFromAtlas3D(
        texture,
        transform,
        uvOffset,
        uvSize,
        glm::vec3(size, 1.f),
        color,
        flipHorizontal,
        flipVertical,
        glm::vec3(origin, 0.f)
    );
}

void AYSpriteRenderer::drawSprite3D(GLuint texture,
    const STTransform& transform,
    const glm::vec3& size,
    const glm::vec4& color,
    bool flipHorizontal,
    bool flipVertical,
    const glm::vec3& origin
)
{
    const auto& context = _renderer->getRenderContext();
    glm::mat4 projection = context.currentCamera->getProjectionMatrix();
    glm::mat4 view = context.currentCamera->getViewMatrix();

    glm::mat4 model = transform.getTransformMatrix();
    glm::vec3 originOffset = size * origin;
    model = glm::translate(model, -originOffset);
    model = glm::scale(model, size);

    auto shader = _getBaseShader();

    _device->saveGLState();
    auto stateManager = _device->getGLStateManager();
    stateManager->useProgram(shader);
    stateManager->bindVertexArray(_vao);

    // 设置uniform
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(glGetUniformLocation(shader, "spriteColor"), 1, glm::value_ptr(color));
    glUniform1f(glGetUniformLocation(shader, "flipH"), flipHorizontal ? -1.0f : 1.0f);
    glUniform1f(glGetUniformLocation(shader, "flipW"), flipVertical ? 1.0f : -1.0f);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, 6);
    _device->restoreGLState();
}

void AYSpriteRenderer::drawSpriteFromAtlas3D(GLuint texture,
    const STTransform& transform,
    const glm::vec2& uvOffset,
    const glm::vec2& uvSize,
    const glm::vec3& size,
    const glm::vec4& color,
    bool flipHorizontal,
    bool flipVertical,
    const glm::vec3& origin
)
{
    const auto& context = _renderer->getRenderContext();
    glm::mat4 projection = context.currentCamera->getProjectionMatrix();
    glm::mat4 view = context.currentCamera->getViewMatrix();

    glm::mat4 model = transform.getTransformMatrix();

    glm::vec3 originOffset = size * glm::vec3(origin.x, origin.y, 0.f);
    model = glm::translate(model, -originOffset);
    model = glm::scale(model, size);

    auto shader = _getAtlasShader();

    _device->saveGLState();
    auto stateManager = _device->getGLStateManager();
    stateManager->useProgram(shader);
    stateManager->setDepthTest(false);
    stateManager->bindVertexArray(_vao);

    // 设置uniform
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(glGetUniformLocation(shader, "spriteColor"), 1, glm::value_ptr(color));
    glUniform2fv(glGetUniformLocation(shader, "uvOffset"), 1, glm::value_ptr(uvOffset));
    glUniform2fv(glGetUniformLocation(shader, "uvSize"), 1, glm::value_ptr(uvSize));
    glUniform1f(glGetUniformLocation(shader, "flipH"), flipHorizontal ? -1.0f : 1.0f);
    glUniform1f(glGetUniformLocation(shader, "flipW"), flipVertical ? 1.0f : -1.0f);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, 6);
    _device->restoreGLState();
}


void AYSpriteRenderer::_initBuffers()
{
    // 顶点数据 (位置 + 纹理坐标)
    float vertices[] = {
        // 位置          // 纹理坐标
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f
    };

    // 使用AYRenderDevice创建VBO
    _vbo = _device->createVertexBuffer(vertices, sizeof(vertices));

    // 创建VAO
    _vao = _device->createVertexArray();

    _device->saveGLState();
    auto stateManager = _device->getGLStateManager();
    stateManager->bindVertexArray(_vao);

    // 绑定VBO并设置顶点属性
    stateManager->bindBuffer(GL_ARRAY_BUFFER, _vbo);

    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // 纹理坐标属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    _device->restoreGLState();
}

void AYSpriteRenderer::_loadSpriteRendererConfigINI()
{
    _config.loadFromFile(_configPath, AYConfigWrapper::ConfigType::INI);

    _baseShader = _config.get<std::string>("shader name.base", std::string("SpriteBaseShader"));
    _atlasShader = _config.get<std::string>("shader name.atlas", std::string("SpriteAtlasShader"));
    _baseVertex = _config.get<std::string>("shader path.base_vertex",
        AYPath::Engine::getPresetShaderPath() + std::string("SpriteRenderer/sprite.vert"));
    _atlasVertex = _config.get<std::string>("shader path.atlas_vertex",
        AYPath::Engine::getPresetShaderPath() + std::string("SpriteRenderer/sprite_atlas.vert"));
    _baseFragment = _config.get<std::string>("shader path.atlas_fragment",
        AYPath::Engine::getPresetShaderPath() + std::string("SpriteRenderer/sprite.frag"));
    _atlasFragment = _config.get<std::string>("shader path.atlas_fragment",
        AYPath::Engine::getPresetShaderPath() + std::string("SpriteRenderer/sprite_atlas.frag"));
}

void AYSpriteRenderer::_saveSpriteRendererConfigINI()
{
    _config.set("shader name.base", _baseShader);
    _config.set("shader name.atlas", _atlasShader);
    _config.set("shader path.base_vertex", _baseVertex);
    _config.set("shader path.atlas_vertex", _atlasVertex);
    _config.set("shader path.atlas_fragment", _baseFragment);
    _config.set("shader path.atlas_fragment", _atlasFragment);

    _config.saveConfig(_configPath);
}

GLuint AYSpriteRenderer::_getBaseShader(bool reload)
{
    return _device->getShaderV(_baseShader, reload, _baseVertex, _baseFragment);
}

GLuint AYSpriteRenderer::_getAtlasShader(bool reload)
{
    return _device->getShaderV(_atlasShader, reload, _atlasVertex, _atlasFragment);
}

