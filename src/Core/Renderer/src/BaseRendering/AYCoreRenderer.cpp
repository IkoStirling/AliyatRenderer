#include "BaseRendering/AYCoreRenderer.h"
#include "AYRenderer.h"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp> ​​
#include "AYPath.h"

#define AY_CHECK_GL_ERROR(context) \
    do { \
        GLenum err; \
        const char* errContext = (context); \
        while ((err = glGetError()) != GL_NO_ERROR) { \
            const char* errStr = ""; \
            switch(err) { \
                case GL_INVALID_ENUM:      errStr = "GL_INVALID_ENUM"; break; \
                case GL_INVALID_VALUE:     errStr = "GL_INVALID_VALUE"; break; \
                case GL_INVALID_OPERATION: errStr = "GL_INVALID_OPERATION"; break; \
                case GL_OUT_OF_MEMORY:    errStr = "GL_OUT_OF_MEMORY"; break; \
                case GL_INVALID_FRAMEBUFFER_OPERATION: errStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break; \
                default:                   errStr = "UNKNOWN_ERROR"; break; \
            } \
            std::cerr << "[OpenGL Error] " << errStr << " (0x" << std::hex << err << ") " \
                      << "in " << errContext << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            if (err == GL_OUT_OF_MEMORY) { \
                std::cerr << "Fatal OpenGL error: Out of memory" << std::endl; \
                std::terminate(); \
            } \
        } \
    } while (0)

std::string mat4ToStringPretty(const glm::mat4& matrix) {
    std::ostringstream oss;
    oss << "[\n";
    for (int i = 0; i < 4; ++i) {
        oss << "  [";
        for (int j = 0; j < 4; ++j) {
            oss << matrix[i][j];
            if (j < 3) oss << ", ";
        }
        oss << "]";
        if (i < 3) oss << ",";
        oss << "\n";
    }
    oss << "]";
    return oss.str();
}

AYCoreRenderer::AYCoreRenderer(AYRenderDevice* device, AYRenderer* renderer):
	_device(device),
	_renderer(renderer),
    _configPath(AYPath::Engine::getPresetConfigPath() + "Renderer/CoreRenderer/config.ini")
{
    _loadCoreRendererConfigINI();
    _setupDebugShader();

    for (auto& batch : _batches)
    {
        batch.vertices.reserve(_vertexBufferSize);
        batch.instanceGroups.reserve(_instanceBufferSize);
    }
}

AYCoreRenderer::~AYCoreRenderer()
{
    _saveCoreRendererConfigINI();
}

void AYCoreRenderer::ensureVertexBufferCapacity(size_t requiredVertices) 
{
    if (requiredVertices > _vertexBufferSize) {
        // 按vector类似方案扩容
        _vertexBufferSize = std::max(_vertexBufferSize * 3 / 2, requiredVertices);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER,
            _vertexBufferSize * sizeof(VertexInfo),
            nullptr,  // 只分配空间，不初始化数据
            GL_DYNAMIC_DRAW);

        AY_CHECK_GL_ERROR("Vertex buffer resize failed");
    }
}

void AYCoreRenderer::ensureInstanceBufferCapacity(size_t requiredInstances) 
{
    if (requiredInstances * sizeof(glm::mat4) > _instanceBufferSize) {
        _instanceBufferSize = std::max(_instanceBufferSize * 3 / 2,
            requiredInstances * sizeof(glm::mat4));

        glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
            _instanceBufferSize,
            nullptr,
            GL_DYNAMIC_DRAW);

        AY_CHECK_GL_ERROR("Instance buffer resize failed");
    }
}

void AYCoreRenderer::ensureIndexBufferCapacity(size_t requiredIndices)
{
}

void AYCoreRenderer::uploadVertexData(const std::vector<VertexInfo>& vertices) 
{
    if (vertices.empty()) return;

    ensureVertexBufferCapacity(vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // 孤儿化缓冲区（抛弃旧数据）
    glBufferData(GL_ARRAY_BUFFER,
        _vertexBufferSize,
        nullptr,
        GL_DYNAMIC_DRAW);

    // 然后上传新数据
    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        vertices.size() * sizeof(VertexInfo),
        vertices.data());

    AY_CHECK_GL_ERROR("Upload Vertex Data failed");
}

void AYCoreRenderer::uploadInstanceData(const std::vector<glm::mat4>& instances) 
{
    if (instances.empty()) return;

    ensureInstanceBufferCapacity(instances.size());

    glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);

    glBufferData(GL_ARRAY_BUFFER,
        _instanceBufferSize,
        nullptr,
        GL_DYNAMIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        instances.size() * sizeof(glm::mat4),
        instances.data());

    AY_CHECK_GL_ERROR("Upload Instance Data failed");
}

void AYCoreRenderer::uploadIndexData(const std::vector<uint32_t>& indices)
{
}



void AYCoreRenderer::drawLine2D(const VertexInfo& start, const VertexInfo& end, Space space)
{
    switchBatchDraw(space, PrimitiveType::Lines);
    _currentBatch->vertices.push_back(start);
    _currentBatch->vertices.push_back(end);
}

void AYCoreRenderer::drawLine2D(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color)
{
    drawLine2D({{start,0},color}, { {end,0},color }, Space::Screen);
}

void AYCoreRenderer::drawTriangle(const VertexInfo& p1,
    const VertexInfo& p2,
    const VertexInfo& p3,
    Space space)
{
    switchBatchDraw(space, PrimitiveType::Triangles);
    _currentBatch->vertices.push_back(p1);
    _currentBatch->vertices.push_back(p2);
    _currentBatch->vertices.push_back(p3);
}

void AYCoreRenderer::drawTriangle(const VertexInfo* vertices,
    const std::vector<std::array<int, 3>>& indices,
    Space space)
{
    if (!vertices)
        return;
    switchBatchDraw(space, PrimitiveType::Triangles);
    for (auto& index : indices)
    {
        _currentBatch->vertices.push_back(vertices[index[0]]);
        _currentBatch->vertices.push_back(vertices[index[1]]);
        _currentBatch->vertices.push_back(vertices[index[2]]);
    }
}

void AYCoreRenderer::drawRect2D(const STTransform& transform,
    const glm::vec2& size,
    const glm::vec4& color,
    bool filled,
    Space space)
{
    switchBatchDraw(space, filled ? PrimitiveType::Triangles : PrimitiveType::Lines);

    
    // 检查是否已有矩形实例组
    bool foundGroup = false;
    for (auto& group : _currentBatch->instanceGroups) {
        if (group.type == InstanceType::Rectangle && group.vertexCount == filled? 6:8) { // 矩形顶点数
            foundGroup = true;
            group.matrices.push_back(transform.getTransformMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f)));
            break;
        }
    }

    if (!foundGroup) {
        InstanceGroup newGroup;
        newGroup.vertexCount = filled ? 6 : 8; // 2个三角形或4条线
        newGroup.baseVertex = _currentBatch->vertices.size();
        newGroup.type = InstanceType::Rectangle;

        // 添加基础顶点
        if (filled) {
            // 两个三角形组成矩形
            VertexInfo baseVertices[6] = {
                // 第一个三角形 - 顺时针
                {{-0.5f,-0.5f,0}, color}, // 左下
                {{0.5f,-0.5f,0}, color},  // 右下
                {{-0.5f,0.5f,0}, color},  // 左上

                // 第二个三角形 - 顺时针 
                {{0.5f,-0.5f,0}, color},  // 右下
                {{0.5f,0.5f,0}, color},   // 右上
                {{-0.5f,0.5f,0}, color}   // 左上
            };
            _currentBatch->vertices.insert(_currentBatch->vertices.end(), baseVertices, baseVertices + 6);
        }
        else {
            // 四条线组成矩形边框
            VertexInfo baseVertices[8] = {
                {{-0.5f,-0.5f,0}, color}, {{0.5f,-0.5f,0}, color}, // 底边
                {{0.5f,-0.5f,0}, color}, {{0.5f,0.5f,0}, color},   // 右边
                {{0.5f,0.5f,0}, color}, {{-0.5f,0.5f,0}, color},  // 顶边
                {{-0.5f,0.5f,0}, color}, {{-0.5f,-0.5f,0}, color} // 左边
            };
            _currentBatch->vertices.insert(_currentBatch->vertices.end(), baseVertices, baseVertices + 8);
        }
        newGroup.matrices.push_back(transform.getTransformMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f)));
        _currentBatch->instanceGroups.push_back(newGroup);
    }
}

void AYCoreRenderer::drawCircle2D(const STTransform& transform,
    float radius,
    const glm::vec4& color,
    int segments,
    bool filled,
    Space space)
{
    glm::mat4 model = transform.getTransformMatrix();

    std::vector<glm::vec3> localCirclePoints;

    const float angleStep = 2.0f * glm::pi<float>() / segments; //   360 / segments
    float angle = 0.0f;

    VertexInfo center = { transform.position, color };

    for (int i = 0; i <= segments; ++i) 
    {
        float angle = i * angleStep;
        localCirclePoints.push_back(glm::vec3(cos(angle), sin(angle), 0.0f));
    }

    for (auto& point : localCirclePoints) 
    {
        glm::vec3 worldPoint = model * glm::vec4(point, 1.0f);
    }

    for (int i = 0; i < localCirclePoints.size() - 1; i++)
    {
        drawLine2D({ localCirclePoints[i],color}, { localCirclePoints[i+1],color },space);
        if (filled)
        {
            drawTriangle(
                center,                                         // 圆心（Z=0，假设 2D）
                { localCirclePoints[i],color },                 // 顶点1
                { localCirclePoints[i + 1],color },             // 顶点2
                space
            );
        }
    }
        
}

void AYCoreRenderer::drawArrow2D(const STTransform& transform, 
    const glm::vec3& from,
    const glm::vec3& to,
    float headSize,
    const glm::vec4& color,
    Space type
)
{
    glm::mat4 model = transform.getTransformMatrix();
    glm::vec3 worldFrom = model * glm::vec4(from, 1.0f);
    glm::vec3 worldTo = model * glm::vec4(to, 1.0f);
    drawLine2D(worldFrom, worldTo, color);

    // arrow head
    glm::vec3 dir = glm::normalize(to - from);
    glm::vec3 perp(-dir.y, dir.x, 0.f);
    glm::vec3 headBase = worldTo - dir * headSize;

    drawLine2D(worldTo, headBase + perp * headSize * 0.5f, color);
    drawLine2D(worldTo, headBase - perp * headSize * 0.5f, color);
}



//void AYCoreRenderer::drawBox3D(const STTransform& transform, const glm::vec3& half_extents, const glm::vec4& color, bool wireframe)
//{
//    const glm::vec3 vertices[8] = {
//    {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
//    {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
//    };
//
//    const int indices[24] = {
//        0,1,1,2,2,3,3,0, // 底面
//        4,5,5,6,6,7,7,4, // 顶面
//        0,4,1,5,2,6,3,7  // 侧面
//    };
//
//    glm::mat4 matrix = transform.getTransformMatrix() *
//        glm::scale(glm::mat4(1.0f), half_extents);
//
//    for (int i = 0; i < 24; i += 2) {
//        glm::vec3 start = matrix * glm::vec4(vertices[indices[i]], 1.0f);
//        glm::vec3 end = matrix * glm::vec4(vertices[indices[i + 1]], 1.0f);
//        //drawLine3D(start, end, color);
//    }
//
//    if (!wireframe) {
//        // 填充面实现...
//    }
//}
//
//void AYCoreRenderer::drawSphere3D(const STTransform& transform, float radius, const glm::vec4& color, int segments)
//{
//}
//
//void AYCoreRenderer::drawCapsule3D(const STTransform& transform, float radius, float half_height, const glm::vec4& color, int segments)
//{
//}
//
//void AYCoreRenderer::drawCylinder3D(const STTransform& transform, float radius, float halfHeight, const glm::vec4& color, int segments)
//{
//}
//
//void AYCoreRenderer::drawMesh3D(const STTransform& transform, const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, const glm::vec4& color, bool wireframe)
//{
//}

void AYCoreRenderer::beginDraw()
{
    _switchVertexBuffer();
    for (auto& batch : _batches)
    {
        batch.vertices.clear();
        batch.instanceGroups.clear();
        batch.projectionMatrix = glm::mat4(1.f);
        batch.viewMatrix = glm::mat4(1.f);
    }
}

void AYCoreRenderer::endDraw() 
{
    endBatchDraw();
    
    flushWithRecover();
}


glm::mat4 AYCoreRenderer::getCurrentProjection(Space space)
{
    glm::mat4 projection;
    auto& context = _renderer->getRenderContext();
    switch (space)
    {
    case AYCoreRenderer::Space::Screen:
        projection = glm::ortho(
            0.0f,
            context.currentCamera->getViewport().z,
            context.currentCamera->getViewport().w,
            0.0f,
            -1.0f,
            1.0f);
        break;
    case AYCoreRenderer::Space::World:
        projection = context.currentCamera->getProjectionMatrix();
        break;
    default:
        break;
    }
    return projection;
}

glm::mat4 AYCoreRenderer::getCurrentView(Space space)
{
    glm::mat4 view;
    auto& context = _renderer->getRenderContext();
    switch (space)
    {
    case AYCoreRenderer::Space::Screen:
        view = glm::mat4(1.f);
        break;
    case AYCoreRenderer::Space::World:
        view = context.currentCamera->getViewMatrix();
        break;
    default:
        break;
    }
    return view;
}

void AYCoreRenderer::switchBatchDraw(Space space, PrimitiveType type)
{
    int index = static_cast<int>(space) * 2 + static_cast<int>(type);
    _currentBatch = &_batches[index];
}

void AYCoreRenderer::endBatchDraw()
{
    for (int i = 0; i < _batches.size(); i++)
    {
        _batches[i].projectionMatrix = getCurrentProjection((Space)(i / 2));
        _batches[i].viewMatrix = getCurrentView((Space)(i / 2));
    }
}

void AYCoreRenderer::flushInstanced(RenderBatch& batch)
{
    if (batch.instanceGroups.empty()) return;

    glBindVertexArray(_vao);
    glUseProgram(_getInstanceShader());

    // 状态设置
    _device->getGLStateManager()->setDepthTest(batch.depthTestEnabled);
    _device->getGLStateManager()->setLineWidth(_lineWidth);

    // 设置矩阵
    glUniformMatrix4fv(glGetUniformLocation(_getInstanceShader(), "projection"),
        1, GL_FALSE, glm::value_ptr(batch.projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(_getInstanceShader(), "view"),
        1, GL_FALSE, glm::value_ptr(batch.viewMatrix));

    // 提交顶点数据
    uploadVertexData(batch.vertices);

    // 为每个实例组单独绘制
    for (auto& group : batch.instanceGroups) 
    {
        uploadInstanceData(group.matrices);

        // 实例化绘制
        glDrawArraysInstanced(batch.primitiveType,
            group.baseVertex,
            group.vertexCount,
            group.matrices.size());
    }

    glBindVertexArray(0);
}

void AYCoreRenderer::flushImmediate(RenderBatch& batch)
{
    glBindVertexArray(_vao);
    glUseProgram(_getBaseShader());
    for (auto& batch : _batches) {
        if (batch.vertices.empty()) continue;

        // 状态设置
        _device->getGLStateManager()->setDepthTest(batch.depthTestEnabled);
        _device->getGLStateManager()->setLineWidth(_lineWidth);

        // 矩阵计算延迟到GPU
        glUniformMatrix4fv(
            glGetUniformLocation(_getBaseShader(), "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(batch.projectionMatrix));
        glUniformMatrix4fv(
            glGetUniformLocation(_getBaseShader(), "view"),
            1,
            GL_FALSE,
            glm::value_ptr(batch.viewMatrix));

        // 提交数据
        uploadVertexData(batch.vertices);

        glDrawArrays(batch.primitiveType, 0, batch.vertices.size());
    }
    glBindVertexArray(0);
}

void AYCoreRenderer::flushWithRecover()
{
    _device->saveGLState();

    for (auto& batch : _batches)
    {
        if (!batch.instanceGroups.empty())
            flushInstanced(batch);
        else
            flushImmediate(batch);
    }

    _device->restoreGLState();
}

void AYCoreRenderer::_loadCoreRendererConfigINI()
{
    _config.loadFromFile(_configPath, AYConfigWrapper::ConfigType::INI);
        
    _instance = _config.get<std::string>("shader name.instance", std::string("CoreInstanceShader"));
    _base = _config.get<std::string>("shader name.base", std::string("CoreBaseShader"));
    _instanceVertexPath = _config.get<std::string>("shader path.instance_vertex",
        AYPath::Engine::getPresetShaderPath() + std::string("CoreRenderer/instance.vert"));
    _instanceFragmentPath = _config.get<std::string>("shader path.instance_fragment",
        AYPath::Engine::getPresetShaderPath() + std::string("CoreRenderer/instance.frag"));
    _baseVertexPath = _config.get<std::string>("shader path.base_vertex",
        AYPath::Engine::getPresetShaderPath() + std::string("CoreRenderer/base.vert"));
    _baseFragmentPath = _config.get<std::string>("shader path.base_fragment",
        AYPath::Engine::getPresetShaderPath() + std::string("CoreRenderer/base.frag"));
}

void AYCoreRenderer::_saveCoreRendererConfigINI()
{
    _config.set("shader name.instance", _instance);
    _config.set("shader name.base", _base);
    _config.set("shader path.instance_vertex", _instanceVertexPath);
    _config.set("shader path.instance_fragment", _instanceFragmentPath);
    _config.set("shader path.base_vertex", _baseVertexPath);
    _config.set("shader path.base_fragment", _baseFragmentPath);

    _config.saveConfig(_configPath);
}

GLuint AYCoreRenderer::_getBaseShader(bool reload)
{
    return _device->getShaderV(_base, reload, _baseVertexPath, _baseFragmentPath);
}

GLuint AYCoreRenderer::_getInstanceShader(bool reload)
{
    return _device->getShaderV(_instance, reload, _instanceVertexPath, _instanceFragmentPath);
}

void AYCoreRenderer::_setupDebugShader()
{
    const size_t INITIAL_VERTEX_BUFFER_SIZE = sizeof(VertexInfo) * _vertexBufferSize;
    const size_t INITIAL_INSTANCE_BUFFER_SIZE = sizeof(glm::mat4) * _instanceBufferSize;

    for (int i = 0; i < 2; i++)
    {
        _vboDouble[i] = _device->createVertexBuffer(nullptr, INITIAL_VERTEX_BUFFER_SIZE, "dynamic");
        _instanceVboDouble[i] = _device->createVertexBuffer(nullptr, INITIAL_VERTEX_BUFFER_SIZE, "dynamic");
    }
    _vao = _device->createVertexArray();

    _vbo = _vboDouble[0];
    _instanceVBO = _instanceVboDouble[0];

    _switchVertexBuffer();
}

void AYCoreRenderer::_switchVertexBuffer()
{
    // 切换当前缓冲区索引
    _vbo = _vboDouble[_currentBufferIndex];
    _instanceVBO = _instanceVboDouble[_currentBufferIndex];
    _currentBufferIndex = 1 - _currentBufferIndex;

    // 更新VAO绑定
    glBindVertexArray(_vao);

    // 重新绑定当前帧的缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, uv));

    // 重新绑定实例缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(3 + i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}