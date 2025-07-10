#include "BaseRendering/AYCoreRenderer.h"
#include "AYRenderer.h"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp> ​​
#include "AYPath.h"
#include "BaseRendering/Graphic/AYGraphicGenerator.h"

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

AYCoreRenderer::AYCoreRenderer(AYRenderDevice* device, AYRenderer* renderer) :
    _device(device),
    _renderer(renderer),
    _configPath(AYPath::Engine::getPresetConfigPath() + "Renderer/CoreRenderer/config.ini")
{
    _loadCoreRendererConfigINI();
    _setupDebugShader();

    _renderBatches.reserve(8);
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
    if (requiredInstances > _instanceBufferSize) {
        _instanceBufferSize = std::max(_instanceBufferSize * 3 / 2,
            requiredInstances);

        glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
            _instanceBufferSize * sizeof(glm::mat4),
            nullptr,
            GL_DYNAMIC_DRAW);

        AY_CHECK_GL_ERROR("Instance buffer resize failed");
    }
}

void AYCoreRenderer::ensureIndexBufferCapacity(size_t requiredIndices)
{
    if (requiredIndices > _indexBufferSize) {
        _indexBufferSize = std::max(_indexBufferSize * 3 / 2,
            _indexBufferSize);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ARRAY_BUFFER,
            _indexBufferSize * sizeof(uint32_t),
            nullptr,
            GL_DYNAMIC_DRAW);

        AY_CHECK_GL_ERROR("Index buffer resize failed");
    }
}

void AYCoreRenderer::uploadVertexData(const std::vector<VertexInfo>& vertices) 
{
    if (vertices.empty()) return;

    ensureVertexBufferCapacity(vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // 孤儿化缓冲区（抛弃旧数据）
    glBufferData(GL_ARRAY_BUFFER,
        _vertexBufferSize * sizeof(VertexInfo),
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
        _instanceBufferSize * sizeof(glm::mat4),
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
    if (indices.empty()) return;

    ensureIndexBufferCapacity(indices.size());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

    // 孤儿化缓冲区（抛弃旧数据）
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        _indexBufferSize * sizeof(uint32_t),
        nullptr,
        GL_DYNAMIC_DRAW);

    // 然后上传新数据
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
        0,
        indices.size() * sizeof(uint32_t),
        indices.data());

    AY_CHECK_GL_ERROR("Upload Index Data failed");
}

AYCoreRenderer::InstanceGroup* AYCoreRenderer::findOrCreateInstanceGroup(
    InstanceType type,
    int expectedVertexCount,
    int expectedIndexCount,
    const glm::mat4& transform)
{
    // 模型矩阵在此函数中添加
    // 查找现有实例组
    for (auto& group : _currentBatch->instanceGroups) {
        if (group.type == type &&
            (expectedVertexCount == -1 || group.vertexCount == expectedVertexCount) &&
            (expectedIndexCount == -1 || group.indexCount == expectedIndexCount)) 
        {
            group.matrices.push_back(transform);
            return &group;
        }
    }

    // 创建并添加新实例组
    InstanceGroup newGroup;
    newGroup.type = type;
    newGroup.baseVertex = _currentBatch->vertices.size();
    newGroup.matrices.push_back(transform);
    _currentBatch->instanceGroups.push_back(newGroup);

    return &_currentBatch->instanceGroups.back();
}

void AYCoreRenderer::addVertexData(const std::vector<VertexInfo>& vertices, InstanceGroup* group)
{
    group->vertexCount = vertices.size();
    _currentBatch->vertices.insert(_currentBatch->vertices.end(), vertices.begin(), vertices.end());
}

void AYCoreRenderer::addIndexData(std::vector<uint32_t>& indices, InstanceGroup* group)
{
    // 调整索引偏移量
    for (auto& index : indices) {
        index += group->baseVertex;
    }

    group->baseIndex = _currentBatch->indices.size();
    group->indexCount = indices.size();
    _currentBatch->indices.insert(_currentBatch->indices.end(), indices.begin(), indices.end());
}

void AYCoreRenderer::drawLine2D(const VertexInfo& start, const VertexInfo& end, Space space)
{
    switchBatchDraw(space, PrimitiveType::Lines, false, STMaterial::Type::None);
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
    switchBatchDraw(space, PrimitiveType::Triangles, false, STMaterial::Type::None);
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
    switchBatchDraw(space, PrimitiveType::Triangles, false, STMaterial::Type::None);
    for (auto& index : indices)
    {
        _currentBatch->vertices.push_back(vertices[index[0]]);
        _currentBatch->vertices.push_back(vertices[index[1]]);
        _currentBatch->vertices.push_back(vertices[index[2]]);
    }
}

void AYCoreRenderer::drawArrow2D(const STTransform& transform,
    const glm::vec3& from,
    const glm::vec3& to,
    float headSize,
    const glm::vec4& color,
    Space space
)
{
    glm::mat4 model = transform.getTransformMatrix();
    glm::vec3 worldFrom = model * glm::vec4(from, 1.0f);
    glm::vec3 worldTo = model * glm::vec4(to, 1.0f);
    drawLine2D(worldFrom, worldTo, color );

    // arrow head
    glm::vec3 dir = glm::normalize(to - from);
    glm::vec3 perp(-dir.y, dir.x, 0.f);
    glm::vec3 headBase = worldTo - dir * headSize;

    drawLine2D(worldTo, headBase + perp * headSize * 0.5f, color);
    drawLine2D(worldTo, headBase - perp * headSize * 0.5f, color);
}


void AYCoreRenderer::drawRect2D(const STTransform& transform,
    const glm::vec2& size,
    uint32_t materialID,
    bool wireframe,
    Space space)
{
    const auto& mat = _renderer->getMaterialManager()->getMaterial(materialID);

    switchBatchDraw(space,
        !wireframe ? PrimitiveType::Triangles : PrimitiveType::Lines,
        true,
        !wireframe ? mat.type : STMaterial::Type::Wireframe,
        materialID);

    auto* group = findOrCreateInstanceGroup(
        InstanceType::Rectangle,
        4, 
        !wireframe ? 6 : 8,
        transform.getTransformMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f))
    );

    if (group->vertexCount == -1) 
    {   
        std::vector<VertexInfo> vertices;
        auto rectPoints = AYGraphicGenerator::createRectV();
        for (const auto& point : rectPoints) {
            vertices.push_back({ point, mat.baseColor });
        }
        addVertexData(vertices, group);

        auto indices = AYGraphicGenerator::createRectI(!wireframe);
        addIndexData(indices, group);
    }
}

void AYCoreRenderer::drawCircle2D(const STTransform& transform,
    float radius,
    uint32_t materialID,
    int segments,
    bool wireframe,
    Space space)
{
    const auto& mat = _renderer->getMaterialManager()->getMaterial(materialID);

    switchBatchDraw(space,
        !wireframe ? PrimitiveType::Triangles : PrimitiveType::Lines,
        true,
        !wireframe ? mat.type : STMaterial::Type::Wireframe,
        materialID);

    auto* group = findOrCreateInstanceGroup(
        InstanceType::Circle,
        segments + 1, // 顶点数
        !wireframe ? segments * 3 : segments * 2, // 索引数
        transform.getTransformMatrix()
    );

    if (group->vertexCount == -1) 
    {
        std::vector<VertexInfo> vertices;
        vertices.push_back({ glm::vec3(0.0f, 0.0f, 0.0f), mat.baseColor }); // 中心点

        auto circlePoints = AYGraphicGenerator::createCircleV(radius, segments);
        for (const auto& point : circlePoints) {
            vertices.push_back({ point, mat.baseColor });
        }
        addVertexData(vertices, group);

        auto indices = AYGraphicGenerator::createCircleI(!wireframe, segments);
        addIndexData(indices, group);
    }
}

void AYCoreRenderer::drawBox3D(const STTransform& transform,
    const glm::vec3& half_extents,
    uint32_t materialID,
    bool wireframe,
    Space space)
{
    const auto& mat = _renderer->getMaterialManager()->getMaterial(materialID);

    switchBatchDraw(space,
        wireframe ? PrimitiveType::Lines : PrimitiveType::Triangles,
        true,
        !wireframe ? mat.type : STMaterial::Type::Wireframe,
        materialID);

    auto* group = findOrCreateInstanceGroup(
        InstanceType::Circle,
        8, // 顶点数
        wireframe ? 24 : 36, // 索引数
        transform.getTransformMatrix()
    );

    if (group->vertexCount == -1)
    {
        std::vector<VertexInfo> vertices;
        auto rectPoints = AYGraphicGenerator::createBoxV(half_extents);
        for (const auto& point : rectPoints) {
            vertices.push_back({ point, mat.baseColor });
        }

        addVertexData(vertices, group);

        auto indices = AYGraphicGenerator::createBoxI(wireframe);
        addIndexData(indices, group);
    }
}

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
    _renderBatches.clear();
    _currentBatch = nullptr;
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

void AYCoreRenderer::switchBatchDraw(Space space,
    PrimitiveType p_type,
    bool useInstanced,
    STMaterial::Type m_type,
    uint32_t materialID)
{
    BatchKey key;
    key.space = space;
    key.primitiveType = (p_type == PrimitiveType::Lines) ? GL_LINES : GL_TRIANGLES;
    key.projectionMatrix = getCurrentProjection(space);
    key.viewMatrix = getCurrentView(space);
    key.depthTestEnabled = true;

    if (space == Space::Screen) {
        key.depthTestEnabled = false; 
    }

    key.useInstanced = useInstanced;
    key.materialType = m_type;
    key.materialID = materialID;

    auto it = _renderBatches.find(key);
    if (it != _renderBatches.end()) {
        _currentBatch = &it->second;
    }
    else {
        // 创建新批次
        RenderBatch newBatch;
        auto result = _renderBatches.emplace(key, std::move(newBatch));
        _currentBatch = &result.first->second;
    }
}

void AYCoreRenderer::endBatchDraw()
{
}

void AYCoreRenderer::flushInstanced(const BatchKey& key, const RenderBatch& batch)
{
    if (batch.vertices.empty())
        return;

    const auto& mat = _renderer->getMaterialManager()->getMaterial(key.materialID);

    _device->saveGLState();
    auto stateManager = _device->getGLStateManager();

    auto shader = _getInstanceShader();

    // 状态设置

    stateManager->bindVertexArray(_vao);
    stateManager->useProgram(shader);
    stateManager->setDepthTest(true);
    stateManager->setDepthMask(true);
    stateManager->setDepthFunc(GL_LESS);
    stateManager->setLineWidth(_lineWidth);
    if (mat.type == STMaterial::Type::Transparent) {
        stateManager->setBlend(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        stateManager->setDepthMask(false);
    }

    // 设置矩阵
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
        1, GL_FALSE, glm::value_ptr(key.projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
        1, GL_FALSE, glm::value_ptr(key.viewMatrix));

    // 设置材质
    glUniform1f(glGetUniformLocation(shader, "u_Metallic"), mat.metallic);
    glUniform1f(glGetUniformLocation(shader, "u_Roughness"), mat.roughness);

    // 提交顶点数据
    uploadVertexData(batch.vertices);

    // 提交索引数据（如果需要）
    if (!batch.indices.empty()) {
        uploadIndexData(batch.indices);
    }

    int curVertex = 0;
    // 为每个实例组单独绘制
    for (auto& group : batch.instanceGroups) 
    {
        //提交实例化模型矩阵
        uploadInstanceData(group.matrices);

        if (group.baseIndex == -1)
        {
            glDrawArraysInstanced(key.primitiveType,  //图元类型
                group.baseVertex,                       //起始位置
                group.vertexCount,                      //顶点数量
                group.matrices.size());                 //绘制数量
        }
        else
        {
            glDrawElementsInstanced(key.primitiveType,//图元类型
                group.indexCount,                       //索引数量，以1个三角形为例，需要三个索引
                GL_UNSIGNED_INT,                        //索引数据类型
                (void*)(group.baseIndex * sizeof(GL_UNSIGNED_INT)),  // 索引偏移量，如果nullptr直接从当前绑定IBO开始
                group.matrices.size());                 //绘制数量
        }
    }

    _device->restoreGLState();
}

void AYCoreRenderer::flushImmediate(const BatchKey& key, const RenderBatch& batch)
{
    if (batch.vertices.empty())
        return;

    _device->saveGLState();
    auto stateManager = _device->getGLStateManager();

    auto shader = _getBaseShader();
    stateManager->bindVertexArray(_vao);
    stateManager->useProgram(shader);
    
    // 状态设置
    _device->getGLStateManager()->setDepthTest(key.depthTestEnabled);
    _device->getGLStateManager()->setLineWidth(_lineWidth);

    // 矩阵计算延迟到GPU
    glUniformMatrix4fv(
        glGetUniformLocation(shader, "projection"),
        1,
        GL_FALSE,
        glm::value_ptr(key.projectionMatrix));
    glUniformMatrix4fv(
        glGetUniformLocation(shader, "view"),
        1,
        GL_FALSE,
        glm::value_ptr(key.viewMatrix));

    // 提交数据
    uploadVertexData(batch.vertices);

    glDrawArrays(key.primitiveType, 0, batch.vertices.size());

    _device->restoreGLState();
}

void AYCoreRenderer::flushWithRecover()
{
    // 先渲染不透明物体
    for (auto& [key, batch] : _renderBatches) {
        const auto& mat = _renderer->getMaterialManager()->getMaterial(key.materialID);
        if (mat.type != STMaterial::Type::Transparent) {
            if (key.useInstanced) flushInstanced(key, batch);
            else flushImmediate(key, batch);
        }
    }

    // 再渲染透明物体
    for (auto& [key, batch] : _renderBatches) {
        const auto& mat = _renderer->getMaterialManager()->getMaterial(key.materialID);
        if (mat.type == STMaterial::Type::Transparent) {
            if (key.useInstanced) flushInstanced(key, batch);
            else flushImmediate(key, batch);
        }
    }
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
    const size_t INITIAL_INDEX_BUFFER_SIZE = 3 * sizeof(uint8_t) * _indexBufferSize;

    for (int i = 0; i < 2; i++)
    {
        _vboDouble[i] = _device->createVertexBuffer(nullptr, INITIAL_VERTEX_BUFFER_SIZE, "dynamic");
        _instanceVboDouble[i] = _device->createVertexBuffer(nullptr, INITIAL_VERTEX_BUFFER_SIZE, "dynamic");
    }
    _vao = _device->createVertexArray();

    _vbo = _vboDouble[0];
    _instanceVBO = _instanceVboDouble[0];

    _ibo = _device->createIndexBuffer(nullptr, INITIAL_INDEX_BUFFER_SIZE);
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

size_t AYCoreRenderer::_computeBatchKeyHash(const BatchKey& key)
{
    size_t seed = 0;

    // 哈希枚举值
    seed ^= std::hash<int>()(static_cast<int>(key.space)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<int>()(static_cast<int>(key.primitiveType)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    // 哈希布尔值
    seed ^= std::hash<bool>()(key.useInstanced) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<bool>()(key.depthTestEnabled) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    // 哈希矩阵（使用 FloatHash）
    auto hashMatrix = [](const glm::mat4& mat) {
        size_t matrixSeed = 0;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                uint32_t bits;
                std::memcpy(&bits, &mat[i][j], sizeof(float));
                matrixSeed ^= std::hash<uint32_t>()(bits) + 0x9e3779b9 + (matrixSeed << 6) + (matrixSeed >> 2);
            }
        }
        return matrixSeed;
        };

    seed ^= hashMatrix(key.projectionMatrix) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hashMatrix(key.viewMatrix) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return seed;
}