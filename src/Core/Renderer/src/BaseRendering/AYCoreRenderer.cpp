#include "BaseRendering/AYCoreRenderer.h"
#include "AYRenderer.h"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp> ​​
#include "AYPath.h"

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
    _configPath("@config/Renderer/CoreRenderer/config.ini")
{
    _loadCoreRendererConfigINI();
    _setupDebugShader();

    _renderBatches.reserve(8);
}

AYCoreRenderer::~AYCoreRenderer()
{
}

void AYCoreRenderer::shutdown()
{
    _saveCoreRendererConfigINI();
}

bool AYCoreRenderer::ensureVertexBufferCapacity(size_t requiredVertices)
{
    if (requiredVertices > _vertexBufferSize) {
        // 按vector类似方案扩容
        _vertexBufferSize = std::max(_vertexBufferSize * 2, requiredVertices);

        _device->getGLStateManager()->bindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER,
            _vertexBufferSize * sizeof(VertexInfo),
            nullptr,  // 只分配空间，不初始化数据
            GL_DYNAMIC_DRAW);
        return true;

        AY_CHECK_GL_ERROR("Vertex buffer resize failed");
    }
    return false;
}

bool AYCoreRenderer::ensureInstanceBufferCapacity(size_t requiredInstances)
{
    if (requiredInstances > _instanceBufferSize) {
        _instanceBufferSize = std::max(_instanceBufferSize * 2,
            requiredInstances);

        _device->getGLStateManager()->bindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
            _instanceBufferSize * sizeof(glm::mat4),
            nullptr,
            GL_DYNAMIC_DRAW);
        return true;

        AY_CHECK_GL_ERROR("Instance buffer resize failed");
    }
    return false;
}

bool AYCoreRenderer::ensureIndexBufferCapacity(size_t requiredIndices)
{
    if (requiredIndices > _indexBufferSize) {
        _indexBufferSize = std::max(_indexBufferSize * 2,
            _indexBufferSize);

        _device->getGLStateManager()->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            _indexBufferSize * sizeof(uint32_t),
            nullptr,
            GL_DYNAMIC_DRAW);
        return true;

        AY_CHECK_GL_ERROR("Index buffer resize failed");
    }
    return false;
}

void AYCoreRenderer::uploadVertexData(const std::vector<VertexInfo>& vertices) 
{
    if (vertices.empty()) return;

    if (!ensureVertexBufferCapacity(vertices.size()))
    {
        _device->getGLStateManager()->bindBuffer(GL_ARRAY_BUFFER, _vbo);
        // 孤儿化缓冲区（抛弃旧数据）
        glBufferData(GL_ARRAY_BUFFER,
            _vertexBufferSize * sizeof(VertexInfo),
            nullptr,
            GL_DYNAMIC_DRAW);
    }
    // 上传新数据
    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        vertices.size() * sizeof(VertexInfo),
        vertices.data());

    AY_CHECK_GL_ERROR("Upload Vertex Data failed");
}

void AYCoreRenderer::uploadInstanceData(const std::vector<glm::mat4>& instances) 
{
    if (instances.empty()) return;

    if (!ensureInstanceBufferCapacity(instances.size()))
    {
        _device->getGLStateManager()->bindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
            _instanceBufferSize * sizeof(glm::mat4),
            nullptr,
            GL_DYNAMIC_DRAW);
    }

    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        instances.size() * sizeof(glm::mat4),
        instances.data());

    AY_CHECK_GL_ERROR("Upload Instance Data failed");
}

void AYCoreRenderer::uploadIndexData(const std::vector<uint32_t>& indices)
{
    if (indices.empty()) return;

    if (!ensureIndexBufferCapacity(indices.size()))
    {
        _device->getGLStateManager()->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        // 孤儿化缓冲区（抛弃旧数据）
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            _indexBufferSize * sizeof(uint32_t),
            nullptr,
            GL_DYNAMIC_DRAW);
    }

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

void AYCoreRenderer::addIndexData(const std::vector<uint32_t>& indices, InstanceGroup* group) const
{
    // 调整索引偏移量
    auto indicesC = indices;
    for (auto& index : indicesC) {
        index += group->baseVertex;
    }

    group->baseIndex = _currentBatch->indices.size();
    group->indexCount = indices.size();
    _currentBatch->indices.insert(_currentBatch->indices.end(), indicesC.begin(), indicesC.end());
}

void AYCoreRenderer::drawLine2D(const VertexInfo& start, const VertexInfo& end, Space space)
{
    switchBatchDraw(space, PrimitiveType::Lines, false);
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
    switchBatchDraw(space, PrimitiveType::Triangles, false);
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
    switchBatchDraw(space, PrimitiveType::Triangles, false);
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
        auto normal = AYGraphicGenerator::create2DN();
        for (const auto& point : rectPoints) {
            vertices.push_back({ point, normal });
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
        auto normal = AYGraphicGenerator::create2DN();
        std::vector<VertexInfo> vertices;
        vertices.push_back({ glm::vec3(0.0f, 0.0f, 0.0f), normal }); // 中心点
        auto circlePoints = AYGraphicGenerator::createCircleV(radius, segments);
        for (const auto& point : circlePoints) {
            vertices.push_back({ point, normal });
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
        wireframe ? 8 : 24, // 顶点数
        wireframe ? 24 : 36, // 索引数
        transform.getTransformMatrix()
    );

    if (group->vertexCount == -1)
    {
        auto vertices = AYGraphicGenerator::createBox(half_extents, wireframe);
        addVertexData(vertices, group);

        auto indices = AYGraphicGenerator::createBoxI(wireframe);
        addIndexData(indices, group);
    }
}

void AYCoreRenderer::drawMesh(const STTransform& transform,
    const STMesh& mesh,
    bool wireframe,
    Space space)
{
    const auto& mat = _renderer->getMaterialManager()->getMaterial(mesh.materialName);

    if (mat.type == STMaterial::Type::Transparent) {
        // 计算网格中心到摄像机的距离
        glm::vec3 worldCenter = transform.getTransformMatrix() * glm::vec4(mesh.center, 1.0f);
        float distance = glm::distance(
            _renderer->getCameraSystem()->getActiveCamera()->getPosition(),
            worldCenter
        );

        // 存入待渲染列表
        _transparentMeshes.emplace_back(transform, mesh, wireframe, space, distance);
    }

    switchBatchDraw(
        space,
        wireframe ? PrimitiveType::Lines : PrimitiveType::Triangles,
        true,  
        !wireframe ? mat.type : STMaterial::Type::Wireframe,
        mat.id       // 可给个默认 materialID 或从材质管理器获取
    );

    auto* group = findOrCreateInstanceGroup(
        InstanceType::Mesh,
        wireframe ? 8 : mesh.vertices.size(), // 顶点数
        wireframe ? 24 : mesh.indices.size(), // 索引数
        transform.getTransformMatrix()
    );

    if (group->vertexCount == -1 && mat.type != STMaterial::Type::Transparent)
    {
        std::vector<VertexInfo> tmp;
        tmp.reserve(mesh.vertices.size());

        for (size_t i = 0; i < mesh.vertices.size(); i++)
        {
            glm::vec3 pos = mesh.vertices[i];
            glm::vec3 normal = (i < mesh.normals.size()) ? mesh.normals[i] : glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec2 uv = (i < mesh.texCoords.size()) ? mesh.texCoords[i] : glm::vec2(0.0f, 0.0f);

            tmp.emplace_back(pos, normal, uv);
        }
        addVertexData(tmp, group);

        addIndexData(mesh.indices, group);
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

uint32_t AYCoreRenderer::getCurrentCameraID(Space space)
{
    uint32_t id = 0;
    auto& context = _renderer->getRenderContext();
    switch (space)
    {
    case AYCoreRenderer::Space::Screen:
        id = _renderer->getCameraSystem()->getCameraID(AYCameraSystem::SCREEN_SPACE_CAMERA);
        break;
    case AYCoreRenderer::Space::World:
        id = context.currentCameraID;
        break;
    default:
        break;
    }
    return id;
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
    key.cameraID = getCurrentCameraID(space);
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

    auto shader = _getInstanceShader(false);

    // 状态设置
    stateManager->setCullFace(true);
    stateManager->bindVertexArray(_vao);
    stateManager->useProgram(shader);
    stateManager->setDepthTest(true);
    stateManager->setDepthMask(true);
    stateManager->setDepthFunc(GL_LESS);
    stateManager->setLineWidth(_lineWidth);
    stateManager->setBlend(false);
    if (mat.type == STMaterial::Type::Transparent) {
        stateManager->setBlend(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        stateManager->setDepthMask(false);
        stateManager->setDepthFunc(GL_LEQUAL);  // 允许等于当前深度的像素通过
    }
    // 设置矩阵
    auto cameraSystem = _renderer->getCameraSystem();
    auto camera = cameraSystem->getCamera(key.cameraID);
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_Projection"),
        1, GL_FALSE, glm::value_ptr(camera->getProjectionMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_View"),
        1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
    // 提交光照
    auto& pos = _renderer->getCameraSystem()->getActiveCamera()->getPosition();
    glUniform3fv(glGetUniformLocation(shader, "u_ViewPos"),
        1, glm::value_ptr(pos));

    auto lightManager = _renderer->getLightManager();
    lightManager->updateLightData();
    lightManager->bindLightData(0); // 绑定到binding point 0

    // 设置材质
    glUniform4fv(glGetUniformLocation(shader, "u_BaseColor"),
        1, glm::value_ptr(mat.baseColor));
    glUniform1f(glGetUniformLocation(shader, "u_Metallic"), mat.metallic);
    glUniform1f(glGetUniformLocation(shader, "u_Roughness"), mat.roughness);
    glUniform1i(glGetUniformLocation(shader, "u_DebugMode"), 0);

    auto texMana = _device->getTextureManager();
    if (!mat.albedoTexture.empty()) {
        auto albedoTextureID = texMana->getTexture(mat.albedoTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoTextureID);
        glUniform1i(glGetUniformLocation(shader, "u_AlbedoTexture"), 0);
        glUniform1i(glGetUniformLocation(shader, "u_UseAlbedoTexture"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shader, "u_UseAlbedoTexture"), 0);
    }

    if (!mat.opacityTexture.empty()) {
        glActiveTexture(GL_TEXTURE1);  // 使用不同的纹理单元
        glBindTexture(GL_TEXTURE_2D, texMana->getTexture(mat.opacityTexture));
        glUniform1i(glGetUniformLocation(shader, "u_OpacityTexture"), 1);
        glUniform1i(glGetUniformLocation(shader, "u_UseOpacityTexture"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shader, "u_UseOpacityTexture"), 0);
    }

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

    auto shader = _getBaseShader(false);
    stateManager->bindVertexArray(_vao);
    stateManager->useProgram(shader);
    
    // 状态设置
    _device->getGLStateManager()->setDepthTest(key.depthTestEnabled);
    _device->getGLStateManager()->setLineWidth(_lineWidth);

    auto cameraSystem = _renderer->getCameraSystem();
    auto camera = cameraSystem->getCamera(key.cameraID);
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_projection"),
        1, GL_FALSE, glm::value_ptr(camera->getProjectionMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_view"),
        1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));

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

}

void AYCoreRenderer::renderTransparentMesh(const TransparentMeshInstance& transMesh)
{

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
    const size_t INITIAL_INDEX_BUFFER_SIZE = 3 * sizeof(uint32_t) * _indexBufferSize;

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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, normal));

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

    seed ^= hashMatrix(key.cameraID) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}