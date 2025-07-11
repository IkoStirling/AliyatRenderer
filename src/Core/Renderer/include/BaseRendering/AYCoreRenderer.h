#pragma once
#include "AYRenderDevice.h"
#include "AYConfigWrapper.h"
#include "STTransform.h"
#include "BaseRendering/Material/STMaterial.h"
#include "BaseRendering/Graphic/AYGraphicGenerator.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <array>

class AYRenderer;
class AYCoreRenderer
{
public:
	AYCoreRenderer(AYRenderDevice* device, AYRenderer* renderer);
	~AYCoreRenderer();

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
    };

    struct InstanceGroup 
    {
        std::vector<glm::mat4> matrices;
        int vertexCount = -1;  // ÿ��ʵ����Ӧ�Ķ�����
        int baseVertex = -1;   // ���㻺���е���ʼλ��
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
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        bool useInstanced;
        bool depthTestEnabled;
        STMaterial::Type materialType;  // ��������
        uint32_t materialID;          // ����Ψһ��ʶ

        bool operator==(const BatchKey& other) const 
        {
            return primitiveType == other.primitiveType &&
                space == other.space &&
                useInstanced == other.useInstanced &&
                depthTestEnabled == other.depthTestEnabled &&
                projectionMatrix == other.projectionMatrix &&
                viewMatrix == other.viewMatrix &&
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

    GLuint _vboDouble[2];   //˫������Ϊ�˽��cpu��gpu���ܲ�ƽ�⣬����һ���ȴ���һ��������
    GLuint _instanceVboDouble[2];
    int _currentBufferIndex = 0;

    GLuint _vao = 0, _vbo = 0, _ibo = 0;
    GLuint _instanceVBO = 0;
public:
    // -------------Base draw Functions-----------------
    // ����ͼ���߼�Ϊ��������任����Ҫ�ģ�ģ�;���*����λ�ã����뻺�棬�ȴ�ͳһ������Ⱦ����
    // ͼ��2D/3Dֻ�����ͼ�ε�������״������������ϵ
    // ��Ļ����ϵ��ʹ������ͶӰ���߶ο�Ȳ��仯���Ҳ�����ͼӰ�죬��UI

    InstanceGroup* findOrCreateInstanceGroup(
        InstanceType type,
        int expectedVertexCount,
        int expectedIndexCount,
        const glm::mat4& transform);

    void addVertexData(const std::vector<VertexInfo>& vertices, InstanceGroup* group);

    void addIndexData(std::vector<uint32_t>& indices, InstanceGroup* group);

    // ����ͼ��
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
        Space space = Space::World);    //û�н���������������

    void drawRect2D(const STTransform& transform,
        const glm::vec2& size,
        uint32_t materialID,
        bool wireframe = false,
        Space space = Space::World);    //ʵ������Ⱦ

    void drawCircle2D(const STTransform& transform,
        float radius,
        uint32_t materialID,
        int segments = 32,
        bool wireframe = false,
        Space space = Space::World);    //ʵ������Ⱦ


    //void drawCapsule2D(const glm::vec2& base, const glm::vec2& tip, float radius, const glm::vec4& color);

    // 3D����ͼ��


    void drawBox3D(const STTransform& transform,
        const glm::vec3& half_extents,
        uint32_t materialID,
        bool wireframe = true,
        Space space = Space::World);

    //void drawSphere3D(const STTransform& transform,
    //    float radius, const glm::vec4& color,
    //    int segments = 16);

    //// ����ͼ��
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

    // �������
    //void drawCross2D(const glm::vec2& position, float size, const glm::vec4& color);
    //void drawCross3D(const glm::vec3& position, float size, const glm::vec4& color);
    //void drawGrid2D(const glm::vec2& center, int divisions, float spacing, const glm::vec4& color);
    //void drawGrid3D(const glm::vec3& center, int divisions, float spacing, const glm::vec3& axisColor);

    // ���߼����ӻ�
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
    
    glm::mat4 getCurrentProjection(Space type);
    glm::mat4 getCurrentView(Space type);


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

    size_t _vertexBufferSize = 1024;    //ͳһΪ���������ֽ�
    size_t _instanceBufferSize = 32;
    size_t _indexBufferSize = 1024;

    void ensureVertexBufferCapacity(size_t requiredVertices);
    void ensureInstanceBufferCapacity(size_t requiredInstances);
    void ensureIndexBufferCapacity(size_t requiredIndices);

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

