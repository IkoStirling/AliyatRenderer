#pragma once
#include "AYRenderDevice.h"
#include "STTransform.h"
#include <glm/glm.hpp>
#include <array>

class AYRenderer;

class AYCoreRenderer
{
public:
	AYCoreRenderer(AYRenderDevice* device, AYRenderer* renderer);
	~AYCoreRenderer();

    enum class Space {
        World = 0,
        Screen = 1
    };
    enum class PrimitiveType {
        Lines = 0,
        Triangles = 1
    };
private:
    struct VertexInfo
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 uv;
    };

    enum class InstanceType {
        Line,
        Triangle,
        Rectangle,
        Cirlcle
    };
    struct InstanceGroup {
        std::vector<glm::mat4> matrices;
        int vertexCount;  // 每个实例对应的顶点数
        int baseVertex;   // 顶点缓冲中的起始位置
        InstanceType type;
    };

    struct RenderBatch
    {
        std::vector<VertexInfo> vertices        = {};
        std::vector<InstanceGroup> instanceGroups = {};
        GLenum primitiveType                    = GL_LINES;
        glm::mat4 projectionMatrix              = glm::mat4(1.0f);
        glm::mat4 viewMatrix                    = glm::mat4(1.0f);
        bool depthTestEnabled                   = false;
    };

    std::array<RenderBatch, 4> _batches = { {
    { {}, {},   GL_LINES,       glm::mat4(1.0f), glm::mat4(1.0f), true  }, // 世界坐标-线框
    { {}, {},   GL_TRIANGLES,   glm::mat4(1.0f), glm::mat4(1.0f), true  }, // 世界坐标-三角形
    { {}, {},   GL_LINES,       glm::mat4(1.0f), glm::mat4(1.0f), false }, // 屏幕空间-线框
    { {}, {},   GL_TRIANGLES,   glm::mat4(1.0f), glm::mat4(1.0f), false }  // 屏幕空间-三角形
    } };

    RenderBatch* _currentBatch = nullptr;
    float _lineWidth = 2.f;



    GLuint _vao = 0, _vbo = 0;
    GLuint _instanceVBO = 0;

    //双缓冲是为了解决cpu与gpu性能不平衡，导致一方等待另一方的问题
    GLuint _vboDouble[2];
    GLuint _instanceVboDouble[2];
    int _currentBufferIndex = 0;
private:
    const std::string _configPath;

    std::string _instance;
    std::string _base;
    std::string _instanceVertexPath;
    std::string _instanceFragmentPath;
    std::string _baseVertexPath;
    std::string _baseFragmentPath;

    GLuint _getBaseShader(bool reload = false);
    GLuint _getInstanceShader(bool reload = false);

    // 缓冲区管理

    void _loadCoreRendererConfigINI();
    void _saveCoreRendererConfigINI();
    size_t _vertexBufferSize      =   1024;
    size_t _instanceBufferSize  =   1024;
    size_t _indexBufferSize     =   1024;

    void ensureVertexBufferCapacity(size_t requiredVertices);
    void ensureInstanceBufferCapacity(size_t requiredInstances);
    void ensureIndexBufferCapacity(size_t requiredIndices); 

    void uploadVertexData(const std::vector<VertexInfo>& vertices);
    void uploadInstanceData(const std::vector<glm::mat4>& instances);
    void uploadIndexData(const std::vector<uint32_t>& indices); 

public:
    // -------------Base draw Functions-----------------
    // 所有图形逻辑为：将顶点变换成需要的（模型矩阵*顶点位置）存入缓存，等待统一批次渲染处理
    // 图形2D/3D只代表该图形的物理形状，不代表坐标系
    // 屏幕坐标系将使用正交投影，线段宽度不变化，且不受视图影响，即UI
    // 基础图形

    void drawLine2D(const VertexInfo& start, const VertexInfo& end, Space space);
    void drawLine2D(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color);
    void drawTriangle(const VertexInfo& p1,
        const VertexInfo& p2,
        const VertexInfo& p3,
        Space space);
    void drawTriangle(const VertexInfo* vertices,
        const std::vector<std::array<int,3>>& indices,
        Space space);

    void drawRect2D(const STTransform& transform,
        const glm::vec2& size,
        const glm::vec4& color,
        bool filled = false,
        Space space = Space::World);

    void drawCircle2D(const STTransform& transform,
        float radius,
        const glm::vec4& color,
        int segments = 32,
        bool filled = false,
        Space space = Space::World);

    void drawArrow2D(const STTransform& transform, 
        const glm::vec3& from,
        const glm::vec3& to,
        float headSize,
        const glm::vec4& color,
        Space space = Space::World);

    //void drawCapsule2D(const glm::vec2& base, const glm::vec2& tip, float radius, const glm::vec4& color);

    // 3D基础图形


    //void drawBox3D(const STTransform& transform,
    //    const glm::vec3& half_extents,
    //    const glm::vec4& color,
    //    bool wireframe = true);

    //void drawSphere3D(const STTransform& transform,
    //    float radius, const glm::vec4& color,
    //    int segments = 16);

    //// 复合图形
    //void drawCapsule3D(const STTransform& transform,
    //    float radius,
    //    float half_height,
    //    const glm::vec4& color,
    //    int segments = 8);

    //void drawCylinder3D(const STTransform& transform,
    //    float radius,
    //    float halfHeight,
    //    const glm::vec4& color,
    //    int segments = 12);

    //void drawMesh3D(const STTransform& transform,
    //    const std::vector<glm::vec3>& vertices,
    //    const std::vector<uint32_t>& indices,
    //    const glm::vec4& color,
    //    bool wireframe = false);

    //void drawArrow3D(const glm::vec3& from, const glm::vec3& to, float headSize, const glm::vec4& color);

    // 辅助标记
    //void drawCross2D(const glm::vec2& position, float size, const glm::vec4& color);
    //void drawCross3D(const glm::vec3& position, float size, const glm::vec4& color);
    //void drawGrid2D(const glm::vec2& center, int divisions, float spacing, const glm::vec4& color);
    //void drawGrid3D(const glm::vec3& center, int divisions, float spacing, const glm::vec3& axisColor);

    // 射线检测可视化
    //void drawRay2D(const glm::vec2& origin, const glm::vec2& direction, float length, const glm::vec4& color);
    //void drawRay3D(const glm::vec3& origin, const glm::vec3& direction, float length, const glm::vec4& color);

    // 批量绘制优化
    void beginDraw();
    void endDraw();
    void switchBatchDraw(Space space, PrimitiveType type);
    void endBatchDraw();
    void flushInstanced(RenderBatch& batch);
    void flushImmediate(RenderBatch& batch);
    void flushWithRecover();
    
    glm::mat4 getCurrentProjection(Space type);
    glm::mat4 getCurrentView(Space type);

private:
    AYRenderDevice* _device;
    AYRenderer* _renderer;

    void _setupDebugShader();
    void _switchVertexBuffer();
};