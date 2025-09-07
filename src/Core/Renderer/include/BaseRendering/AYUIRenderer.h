﻿#pragma once
#include "AYRenderDevice.h"
#include "Event_UIPackages.h"
#include "Event_InputPackages.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

class AYRenderer;
class AYUIEventHandler;
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

    void setOnClicked(uint32_t elementId, std::function<void()> callback);
    void setOnHovered(uint32_t elementId, std::function<void()> callback);
    void setOnUnhovered(uint32_t elementId, std::function<void()> callback);
    void setOnPressed(uint32_t elementId, std::function<void()> callback);
    void setOnReleased(uint32_t elementId, std::function<void()> callback);

    bool isInsideElement(uint32_t elementId, const glm::vec2& position);

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
        std::vector<bool> visibles;            // 是否可见
        std::vector<bool> dirtyFlags;          // 脏标记
        std::vector<uint32_t> elementIds;      // 对应的元素ID

        // 单个批次依靠以下两个作为判断
        GLuint textureId;          
        bool is3D = false;

        // 批处理信息
        uint32_t vertexOffset = 0;
        uint32_t vertexCount = 0;
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;

        float minDepth = 0.0f;
        float maxDepth = 0.0f;
    };

    struct UIRenderData {
        glm::vec3 position;
        glm::vec3 size;
        glm::vec4 color;
        GLuint texture;
        bool visible;
        bool is3D;
        glm::vec3 rotation;
        glm::vec3 scale;
        glm::vec2 anchor = { 0.0f, 0.0f };
        float depth;
        bool dirty;
    };

    struct UIInteractionState {
        bool isHovered;
        bool isPressed;
        bool isFocused;
        bool interactive;
        bool enabled;
    };

    struct UICallback {
        std::function<void()> onClicked;      // 点击时触发
        std::function<void()> onHovered;      // 鼠标悬停进入时触发
        std::function<void()> onUnhovered;    // 鼠标悬停离开时触发
        std::function<void()> onPressed;      // 鼠标按下时触发
        std::function<void()> onReleased;     // 鼠标释放时触发（不一定点击）
    };

    // UI元素数据结构
    struct UIElement {
        uint32_t id;
        UIRenderData renderData;
        UIInteractionState interaction;
        UICallback callback;
        std::string text;
        glm::vec4 textColor = glm::vec4(1,1,0,1);
        float textScale;

        // === 交互配置（样式 / 状态颜色，可选）===
        glm::vec4 hoverColor;     // 悬停时颜色（可选，根据需要使用）
        glm::vec4 pressedColor;   // 按下时颜色
        glm::vec4 disabledColor;  // 禁用时颜色
    };

    // 顶点结构
    struct UIVertex {
        glm::vec3 position;
        glm::vec2 texCoord;
        glm::vec4 color;
        glm::vec3 normal; //3D 扩展
    };

    UIBatch* getTargetBatch(const UIElement& element);
    UIBatch* getTargetBatch(GLuint textureId, bool is3D);

    void setupUIBuffers();
    void updateElementVertices(uint32_t elementId);
    void updateTextVertices(uint32_t elementId);
    void flushDirtyElements();
    void uploadVertexData();    

    void debugRender() {
        std::cout << "=== UI Debug Info ===" << std::endl;
        std::cout << "Total Elements: " << _uiElements.size() << std::endl;
        std::cout << "Total Batches: " << _uiBatches.size() << std::endl;

        size_t totalVertices = 0;
        size_t totalIndices = 0;

        for (const auto& batch : _uiBatches) {
            std::cout << "  Batch " << &batch - &_uiBatches[0] << ": "
                << "texture=" << batch.textureId
                << ", 3D=" << batch.is3D
                << ", vertices=" << batch.vertexCount
                << ", indices=" << batch.indexCount
                << std::endl;

            totalVertices += batch.vertexCount;
            totalIndices += batch.indexCount;
        }

        std::cout << "Total Vertices: " << totalVertices << std::endl;
        std::cout << "Total Indices: " << totalIndices << std::endl;
        std::cout << "Dirty Elements: " << _dirtyElements.size() << std::endl;
        std::cout << "=====================" << std::endl;
    }

    std::unique_ptr<AYUIEventHandler> _handler;
    AYRenderDevice* _device;
    AYRenderer* _renderer;

    GLuint _uiVAO;
    GLuint _uiVBO;
    GLuint _uiEBO; // 添加元素缓冲对象用于索引绘制

    friend class AYUIEventHandler;
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