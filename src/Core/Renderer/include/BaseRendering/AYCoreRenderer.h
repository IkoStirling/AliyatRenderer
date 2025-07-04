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
        int vertexCount;  // ÿ��ʵ����Ӧ�Ķ�����
        int baseVertex;   // ���㻺���е���ʼλ��
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
    { {}, {},   GL_LINES,       glm::mat4(1.0f), glm::mat4(1.0f), true  }, // ��������-�߿�
    { {}, {},   GL_TRIANGLES,   glm::mat4(1.0f), glm::mat4(1.0f), true  }, // ��������-������
    { {}, {},   GL_LINES,       glm::mat4(1.0f), glm::mat4(1.0f), false }, // ��Ļ�ռ�-�߿�
    { {}, {},   GL_TRIANGLES,   glm::mat4(1.0f), glm::mat4(1.0f), false }  // ��Ļ�ռ�-������
    } };

    RenderBatch* _currentBatch = nullptr;
    float _lineWidth = 2.f;



    GLuint _vao = 0, _vbo = 0;
    GLuint _instanceVBO = 0;

    //˫������Ϊ�˽��cpu��gpu���ܲ�ƽ�⣬����һ���ȴ���һ��������
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

    // ����������

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
    // ����ͼ���߼�Ϊ��������任����Ҫ�ģ�ģ�;���*����λ�ã����뻺�棬�ȴ�ͳһ������Ⱦ����
    // ͼ��2D/3Dֻ�����ͼ�ε�������״������������ϵ
    // ��Ļ����ϵ��ʹ������ͶӰ���߶ο�Ȳ��仯���Ҳ�����ͼӰ�죬��UI
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

    // 3D����ͼ��


    //void drawBox3D(const STTransform& transform,
    //    const glm::vec3& half_extents,
    //    const glm::vec4& color,
    //    bool wireframe = true);

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

    // ���������Ż�
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