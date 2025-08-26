#pragma once
#include "AYRenderDevice.h"
#include "AYConfigWrapper.h"
#include "STTransform.h"
#include "STMaterial.h"
#include "STMesh.h"
#include "BaseRendering/Graphic/AYGraphicGenerator.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <array>


/*
 * 1、移除矩阵比较，使用相机id判断批次，需要CameraSystem支持
 * 2、使用glmap buffer 替代缓冲区管理操作
 * 3、合并所有需要上传的实例组
 * 4、使用着色器缓存，类似当前的batchkey操作 (已支持），考虑使用ubo传递相机位置矩阵等共享信息
 * 5、透明物体排序
 * 6、使用不同vao进行渲染，而非相同vao然后切换属性
 * 7、使用内存池替代频繁的vector拷贝操作
 * 
*/

class AYRenderer;
class AYCoreRenderer
{
public:
	AYCoreRenderer(AYRenderDevice* device, AYRenderer* renderer);
	~AYCoreRenderer();
    void shutdown();

    enum class Space 
    {
        World = 0,
        Screen = 1
    };
    enum class PrimitiveType 
    {
        Lines = 0,
        Triangles = 1
    };

private:


    enum class InstanceType 
    {
        Line,
        Triangle,
        Rectangle,
        Circle,
        Box,
        Mesh
    };

    struct InstanceGroup 
    {
        std::vector<glm::mat4> matrices;
        int vertexCount = -1;  // 每个实例对应的顶点数
        int baseVertex = -1;   // 顶点缓冲中的起始位置
        int indexCount = -1;
        int baseIndex = -1;
        InstanceType type;
    };

    struct RenderBatch
    {
        std::vector<VertexInfo> vertices        = {};
        std::vector<InstanceGroup> instanceGroups = {};
        std::vector<uint32_t> indices           = {};
    };

    struct BatchKey
    {
        Space space;
        GLenum primitiveType;
        //glm::mat4 projectionMatrix;
        //glm::mat4 viewMatrix;
        bool useInstanced;
        bool depthTestEnabled;
        STMaterial::Type materialType;  // 材质类型
        uint32_t materialID;            // 材质唯一标识
        uint32_t cameraID;              // 相机唯一标识

        bool operator==(const BatchKey& other) const 
        {
            return primitiveType == other.primitiveType &&
                space == other.space &&
                useInstanced == other.useInstanced &&
                depthTestEnabled == other.depthTestEnabled &&
                cameraID == other.cameraID &&
                //projectionMatrix == other.projectionMatrix &&
                //viewMatrix == other.viewMatrix &&
                materialType == other.materialType &&
                materialID == other.materialID
                ;
        }
    };

    struct BatchKeyHasher
    {
        size_t operator()(const BatchKey& key) const
        {
            return AYCoreRenderer::_computeBatchKeyHash(key);
        }
    };

    std::unordered_map<BatchKey, RenderBatch, BatchKeyHasher> _renderBatches;

    RenderBatch* _currentBatch = nullptr;

    GLuint _vboDouble[2];   //双缓冲是为了解决cpu与gpu性能不平衡，导致一方等待另一方的问题
    GLuint _instanceVboDouble[2];
    int _currentBufferIndex = 0;

    GLuint _vao = 0, _vbo = 0, _ibo = 0;
    GLuint _instanceVBO = 0;
public:
    // -------------Base draw Functions-----------------
    // 所有图形逻辑为：将顶点变换成需要的（模型矩阵*顶点位置）存入缓存，等待统一批次渲染处理
    // 图形2D/3D只代表该图形的物理形状，不代表坐标系
    // 屏幕坐标系将使用正交投影，线段宽度不变化，且不受视图影响，即UI

    InstanceGroup* findOrCreateInstanceGroup(
        InstanceType type,
        int expectedVertexCount,
        int expectedIndexCount,
        const glm::mat4& transform);

    void addVertexData(const std::vector<VertexInfo>& vertices, InstanceGroup* group);

    void addIndexData(const std::vector<uint32_t>& indices, InstanceGroup* group) const;

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

    void drawArrow2D(const STTransform& transform, 
        const glm::vec3& from,
        const glm::vec3& to,
        float headSize,
        const glm::vec4& color,
        Space space = Space::World);    //没有进行世界坐标适配

    void drawRect2D(const STTransform& transform,
        const glm::vec2& size,
        uint32_t materialID,
        bool wireframe = false,
        Space space = Space::World);    //实例化渲染

    void drawCircle2D(const STTransform& transform,
        float radius,
        uint32_t materialID,
        int segments = 32,
        bool wireframe = false,
        Space space = Space::World);    //实例化渲染


    //void drawCapsule2D(const glm::vec2& base, const glm::vec2& tip, float radius, const glm::vec4& color);

    // 3D基础图形


    void drawBox3D(const STTransform& transform,
        const glm::vec3& half_extents,
        uint32_t materialID,
        bool wireframe = true,
        Space space = Space::World);

    void drawMesh(const STTransform& transform,
        const STMesh& mesh,
        bool wireframe,
        Space space);

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


public:
    //---------------Batch Draw Functions----------------
    void beginDraw();
    void endDraw();
    void endBatchDraw();

private:
    void switchBatchDraw(Space space,
        PrimitiveType p_type,
        bool useInstanced,
        STMaterial::Type m_type = STMaterial::Type::None,
        uint32_t materialID = 0);
    void flushInstanced(const BatchKey& key, const RenderBatch& batch);
    void flushImmediate(const BatchKey& key, const RenderBatch& batch);
    void flushWithRecover();

    //glm::mat4 getCurrentProjection(Space type);
    //glm::mat4 getCurrentView(Space type);
    uint32_t getCurrentCameraID(Space space);

    struct TransparentMeshInstance {
        STTransform transform;  // 网格的世界变换
        STMesh mesh;           // 网格数据
        bool wireframe;        // 是否线框模式
        Space space;           // 空间类型（World/Screen）
        float distanceToCamera;// 到摄像机的距离（用于排序）
    };

    void renderTransparentMesh(const TransparentMeshInstance& transMesh);
    std::vector<TransparentMeshInstance> _transparentMeshes;  // 存储透明网格的容器

private:
    //---------------Configs----------------
    AYConfigWrapper _config;
    const std::string _configPath;
    void _loadCoreRendererConfigINI();
    void _saveCoreRendererConfigINI();

    std::string _instance;
    std::string _base;
    std::string _instanceVertexPath;
    std::string _instanceFragmentPath;
    std::string _baseVertexPath;
    std::string _baseFragmentPath;

    float _lineWidth = 5.f;
    void setLineWidth(float line_width) { _lineWidth = line_width; }

    GLuint _getBaseShader(bool reload = false);
    GLuint _getInstanceShader(bool reload = false);

private:
    // -------------Cache Managment-----------------

    size_t _vertexBufferSize = 1024;    //统一为数量而非字节
    size_t _instanceBufferSize = 32;
    size_t _indexBufferSize = 1024;

    bool ensureVertexBufferCapacity(size_t requiredVertices);
    bool ensureInstanceBufferCapacity(size_t requiredInstances);
    bool ensureIndexBufferCapacity(size_t requiredIndices);

    void uploadVertexData(const std::vector<VertexInfo>& vertices);
    void uploadInstanceData(const std::vector<glm::mat4>& instances);
    void uploadIndexData(const std::vector<uint32_t>& indices);
private:
    AYRenderDevice* _device;
    AYRenderer* _renderer;

    void _setupDebugShader();
    void _switchVertexBuffer();
    static size_t _computeBatchKeyHash(const BatchKey& key);
};

