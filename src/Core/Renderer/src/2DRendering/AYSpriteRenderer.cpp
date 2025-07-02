#include "2DRendering/AYSpriteRenderer.h"
#include "AYRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> ​​

const char* SPRITE_VERTEX_SHADER = R"(
    #version 460 core
    layout (location = 0) in vec2 aPos;
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
        gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
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
        layout (location = 0) in vec2 aPos;
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
            gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
        }
    )";

AYSpriteRenderer::AYSpriteRenderer(AYRenderDevice* device, AYRenderer* renderer):
    _device(device),
    _renderer(renderer)
{
    init();
}

AYSpriteRenderer::~AYSpriteRenderer()
{
    if (_shaderProgram) {
        glDeleteProgram(_shaderProgram);
    }
    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
    }
    if (_vbo) {
        glDeleteBuffers(1, &_vbo);
    }
}

void AYSpriteRenderer::init()
{
    _initShader();
    _initBuffers();
    _initAtlasShader();
}

void AYSpriteRenderer::drawSprite(GLuint texture,
    const glm::vec2& position,
    const glm::vec2& size,
    float rotation,
    const glm::vec4& color,
    bool flipHorizontal,
    bool flipVertical,
    const glm::vec2& origin
)
{
    const auto& context = _renderer->getRenderContext();
    glm::mat4 projection = context.currentCamera->getProjectionMatrix();
    glm::mat4 view = context.currentCamera->getViewMatrix();

    glm::vec2 originOffset = size * origin;
    glm::mat4 model = _prepareModel(position - originOffset, size, rotation, origin);

    // 使用着色器
    glUseProgram(_shaderProgram);

    // 设置uniform
    glUniformMatrix4fv(glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(glGetUniformLocation(_shaderProgram, "spriteColor"), 1, glm::value_ptr(color));
    glUniform1f(glGetUniformLocation(_shaderProgram, "flipH"), flipHorizontal ? -1.0f : 1.0f);
    glUniform1f(glGetUniformLocation(_shaderProgram, "flipW"), flipVertical ? -1.0f : 1.0f);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 绘制
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void AYSpriteRenderer::drawSpriteFromAtlas(GLuint texture,
    const glm::vec2& position,
    const glm::vec2& size,
    const glm::vec2& uvOffset,
    const glm::vec2& uvSize, 
    float rotation, 
    const glm::vec4& color,
    bool flipHorizontal,
    bool flipVertical,
    const glm::vec2& origin
)
{
    const auto& context = _renderer->getRenderContext();
    glm::mat4 projection = context.currentCamera->getProjectionMatrix();
    glm::mat4 view = context.currentCamera->getViewMatrix();

    glm::vec2 originOffset = size * origin;
    glm::mat4 model = _prepareModel(position - originOffset, size, rotation, origin);

    // 使用图集着色器
    glUseProgram(_atlasShaderProgram);

    // 设置uniform
    glUniformMatrix4fv(glGetUniformLocation(_atlasShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(_atlasShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(_atlasShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(glGetUniformLocation(_atlasShaderProgram, "spriteColor"), 1, glm::value_ptr(color));
    glUniform2fv(glGetUniformLocation(_atlasShaderProgram, "uvOffset"), 1, glm::value_ptr(uvOffset));
    glUniform2fv(glGetUniformLocation(_atlasShaderProgram, "uvSize"), 1, glm::value_ptr(uvSize));
    glUniform1f(glGetUniformLocation(_shaderProgram, "flipH"), flipHorizontal ? -1.0f : 1.0f);
    glUniform1f(glGetUniformLocation(_shaderProgram, "flipW"), flipVertical ? -1.0f : 1.0f);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 绘制
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void AYSpriteRenderer::setViewportSize(int width, int height)
{
    _viewportWidth = width;
    _viewportHeight = height;
}

glm::mat4 AYSpriteRenderer::_prepareModel(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec2& origin)
{
    // 准备模型矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    // 应用原点偏移
    model = glm::translate(model, glm::vec3(origin.x * size.x, origin.y * size.y, 0.0f));

    // 应用旋转
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    // 应用原点偏移的反向平移
    model = glm::translate(model, glm::vec3(-origin.x * size.x, -origin.y * size.y, 0.0f));

    // 应用缩放
    model = glm::scale(model, glm::vec3(size, 1.0f));

    // 准备投影矩阵（正交投影 —》 2D渲染）
    return model;
}

glm::mat4 AYSpriteRenderer::_getProjecction()
{
    return glm::ortho(
        0.0f,                       // 视口左边界（x 最小值）
        (float)_viewportWidth,      // 视口右边界（x 最大值）
        (float)_viewportHeight,     // 视口下边界（y 最小值）
        0.0f,                       // 视口上边界（y 最大值） 屏幕坐标系
        -1.0f,                      // 近裁剪面（z 最小值）
        1.0f                        // 远裁剪面（z 最大值）
    );
}

void AYSpriteRenderer::_initShader()
{
    _shaderProgram = _device->createShaderProgram(SPRITE_VERTEX_SHADER, SPRITE_FRAGMENT_SHADER);
}

void AYSpriteRenderer::_initBuffers()
{
    // 顶点数据 (位置 + 纹理坐标)
    float vertices[] = {
        // 位置       // 纹理坐标
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 0.0f,

        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f
    };

    // 使用AYRenderDevice创建VBO
    _vbo = _device->createVertexBuffer(vertices, sizeof(vertices));

    // 创建VAO
    _vao = _device->createVertexArray();
    glBindVertexArray(_vao);

    // 绑定VBO并设置顶点属性
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // 纹理坐标属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AYSpriteRenderer::_initAtlasShader()
{
    _atlasShaderProgram = _device->createShaderProgram(SPRITE_VERTEX_SHADER_ATLAS, SPRITE_FRAGMENT_SHADER);
}

