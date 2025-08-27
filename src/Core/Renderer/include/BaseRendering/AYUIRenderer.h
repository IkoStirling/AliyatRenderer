#pragma once
#include "AYRenderDevice.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

class AYRenderer;
class AYUIRenderer {
public:
    AYUIRenderer(AYRenderDevice* device, AYRenderer* renderer);
    ~AYUIRenderer();

    void init();
    void shutdown();

    // UI元素管理
    uint32_t createRectangle(const glm::vec3& position, const glm::vec3& size,
        const glm::vec4& color, GLuint texture = 0, bool is3D = false,
        const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f));

    uint32_t createText(const std::string& text, const glm::vec3& position,
        const glm::vec4& color, float textScale = 1.0f, bool is3D = false);

    // 修改UI元素
    void setPosition(uint32_t elementId, const glm::vec3& position);
    void setSize(uint32_t elementId, const glm::vec3& size);
    void setColor(uint32_t elementId, const glm::vec4& color);
    void setText(uint32_t elementId, const std::string& text);
    void setVisible(uint32_t elementId, bool visible);
    void setRotation(uint32_t elementId, const glm::vec3& rotation);
    void setScale(uint32_t elementId, const glm::vec3& scale);
    void setIs3D(uint32_t elementId, bool is3D);

    // 渲染
    void beginUIFrame();
    void renderUI();
    void endUIFrame();

private:
    // UIBatch - 将相关的顶点数据和属性数据聚合在一起
    struct UIBatch {
        std::vector<glm::vec3> positions;      // 顶点位置
        std::vector<glm::vec2> texCoords;      // 纹理坐标
        std::vector<glm::vec3> normals;        // 法线
        std::vector<glm::vec4> colors;         // 顶点颜色
        std::vector<GLuint> indices;           // 索引数据
        std::vector<GLuint> textures;          // 每个元素的纹理ID
        std::vector<bool> hasTextures;         // 是否使用纹理
        std::vector<bool> visibles;            // 是否可见
        std::vector<bool> dirtyFlags;          // 脏标记
        std::vector<uint32_t> elementIds;      // 对应的元素ID

        // 批处理信息
        uint32_t vertexOffset = 0;
        uint32_t vertexCount = 0;
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;

        bool is3D = false;
        float minDepth = 0.0f;
        float maxDepth = 0.0f;
    };

    // UI元素数据结构
    struct UIElement {
        uint32_t id;
        glm::vec3 position;
        glm::vec3 rotation; //3D 扩展
        glm::vec3 scale;
        glm::vec3 size;
        glm::vec4 color;
        GLuint texture;
        bool hasTexture;
        bool visible;
        bool dirty;
        std::string text;
        float textScale;
        float depth;

        bool is3D = false;
    };

    // 顶点结构
    struct UIVertex {
        glm::vec3 position;
        glm::vec2 texCoord;
        glm::vec4 color;
        glm::vec3 normal; //3D 扩展
    };

    void setupUIBuffers();
    void updateElementVertices(uint32_t elementId);
    void updateTextVertices(uint32_t elementId);
    void flushDirtyElements();
    void uploadVertexData();    //uc

    AYRenderDevice* _device;
    AYRenderer* _renderer;

    GLuint _uiVAO;
    GLuint _uiVBO;
    GLuint _uiEBO; // 添加元素缓冲对象用于索引绘制

    std::vector<UIBatch> _uiBatches;         // 批处理数据
    std::vector<UIElement> _uiElements;      // UI元素数据

    // 脏元素管理
    std::vector<uint32_t> _dirtyElements;
    std::vector<uint32_t> _dirtyTextElements;

    bool _vertexBufferDirty;

    uint32_t _nextElementId;

    //---------------Configs----------------
    AYConfigWrapper _config;
    const std::string _configPath;
    void _loadUIRendererConfigINI();
    void _saveUIRendererConfigINI();

    std::string _uiShaderName;
    std::string _uiVertexPath;
    std::string _uiFragmentPath;

    GLuint _getUIShader(bool reload = false);
};