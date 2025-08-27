#include "BaseRendering/AYUIRenderer.h"
#include "AYRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

AYUIRenderer::AYUIRenderer(AYRenderDevice* device, AYRenderer* renderer):
    _device(device),
    _renderer(renderer),
    _nextElementId(1),
    _vertexBufferDirty(false) ,
    _configPath("@config/Renderer/UIRenderer/config.ini")
{
    _loadUIRendererConfigINI();
    init();
}

AYUIRenderer::~AYUIRenderer() {
    shutdown();
}

void AYUIRenderer::init() {
    setupUIBuffers();

    // 初始分配
    _uiElements.reserve(100);
    _uiBatches.reserve(10); // 预分配一些批次
    _vertexBufferDirty = true;
}

void AYUIRenderer::shutdown() {
    if (_uiVAO) glDeleteVertexArrays(1, &_uiVAO);
    if (_uiVBO) glDeleteBuffers(1, &_uiVBO);
    if (_uiEBO) glDeleteBuffers(1, &_uiEBO);
    _saveUIRendererConfigINI();
}

uint32_t AYUIRenderer::createRectangle(const glm::vec3& position, const glm::vec3& size,
    const glm::vec4& color, GLuint texture, bool is3D,
    const glm::vec3& rotation, const glm::vec3& scale) 
{
    UIElement element;
    element.id = _nextElementId++;  // 分配唯一ID
    element.position = position;    // 设置位置
    element.size = size;            // 设置尺寸
    element.color = color;          // 设置颜色
    element.texture = texture;      // 设置纹理（可为0表示无纹理）
    element.hasTexture = (texture != 0); // 判断是否使用纹理
    element.visible = true;         // 默认可见
    element.dirty = true;           // 标记为脏（需要首次渲染）
    element.text = "";              // 矩形无文本内容
    element.scale = scale;           // 缩放默认值
    element.rotation = rotation;     // 旋转默认值
    element.is3D = is3D;            // 是否为3D元素

    _uiElements.push_back(element);
    _dirtyElements.push_back(element.id - 1);
    _vertexBufferDirty = true;

    return element.id;
}

uint32_t AYUIRenderer::createText(const std::string& text, const glm::vec3& position,
    const glm::vec4& color, float scale, bool is3D)
{
    UIElement element;
    element.id = _nextElementId++;
    element.position = position;
    element.size = glm::vec3(1); // 文本大小动态计算
    element.color = color;
    element.texture = 0; // 使用字体纹理图集
    element.hasTexture = true;
    element.visible = true;
    element.dirty = true;
    element.text = text;
    element.textScale = scale;
    element.scale = glm::vec3(1.0f);
    element.rotation = glm::vec3(0.0f);
    element.is3D = is3D;

    _uiElements.push_back(element);
    _dirtyTextElements.push_back(element.id - 1);
    _vertexBufferDirty = true;

    return element.id;
}

void AYUIRenderer::setPosition(uint32_t elementId, const glm::vec3& position) {
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].position != position) {
            _uiElements[index].position = position;
            _uiElements[index].dirty = true;
            _dirtyElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::setSize(uint32_t elementId, const glm::vec3& size) {
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].size != size) {
            _uiElements[index].size = size;
            _uiElements[index].dirty = true;
            _dirtyElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::setColor(uint32_t elementId, const glm::vec4& color) {
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].color != color) {
            _uiElements[index].color = color;
            _uiElements[index].dirty = true;
            _dirtyElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::setText(uint32_t elementId, const std::string& text) {
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].text != text) {
            _uiElements[index].text = text;
            _uiElements[index].dirty = true;
            _dirtyTextElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::setVisible(uint32_t elementId, bool visible) {
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].visible != visible) {
            _uiElements[index].visible = visible;
            _uiElements[index].dirty = true;
            _dirtyElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::setRotation(uint32_t elementId, const glm::vec3& rotation)
{
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].rotation != rotation) {
            _uiElements[index].rotation = rotation;
            _uiElements[index].dirty = true;
            _dirtyElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::setScale(uint32_t elementId, const glm::vec3& scale)
{
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].scale != scale) {
            _uiElements[index].scale = scale;
            _uiElements[index].dirty = true;
            _dirtyElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::setIs3D(uint32_t elementId, bool is3D)
{
    if (elementId > 0 && elementId <= _uiElements.size()) {
        uint32_t index = elementId - 1;
        if (_uiElements[index].is3D != is3D) {
            _uiElements[index].is3D = is3D;
            _uiElements[index].dirty = true;
            _dirtyElements.push_back(index);
            _vertexBufferDirty = true;
        }
    }
}

void AYUIRenderer::beginUIFrame() {
    _device->saveGLState();
    auto stateManager = _device->getGLStateManager();
    stateManager->setBlend(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    stateManager->setCullFace(false);
}

void AYUIRenderer::renderUI() {
    // 先处理所有脏元素
    flushDirtyElements();

    // 如果有脏数据，上传到GPU
    if (_vertexBufferDirty) {
        uploadVertexData();
        _vertexBufferDirty = false;
    }

    auto stateManager = _device->getGLStateManager();
    auto uiShader = _getUIShader(true);
    stateManager->useProgram(uiShader);
    stateManager->bindVertexArray(_uiVAO);


    // 渲染所有批次
    for (auto& batch : _uiBatches) {
        if (batch.indexCount == 0) continue;

        // 更新投影矩阵
        auto cameraSystem = _renderer->getCameraSystem();
        IAYCamera* camera;
        glm::mat4 projection;
        glm::mat4 model;
        glm::mat4 view;
        if (!batch.is3D)
        {
            camera = cameraSystem->getCamera(AYCameraSystem::SCREEN_SPACE_CAMERA);
            projection = camera->getProjectionMatrix();
            view = camera->getViewMatrix();
            stateManager->setDepthTest(false);
            model = glm::mat4(1.f); //预留，不做实现
        }
        else
        {
            camera = cameraSystem->getActiveCamera();
            projection = camera->getProjectionMatrix();
            view = camera->getViewMatrix();
            stateManager->setDepthTest(false);
            model = glm::mat4(1.f);
        }
        
        glUniformMatrix4fv(glGetUniformLocation(uiShader, "u_projection"),
            1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(uiShader, "u_model"),
            1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(uiShader, "u_view"),
            1, GL_FALSE, glm::value_ptr(view));

        if (!batch.hasTextures.empty()) { // 简化处理：假设批次内所有元素使用相同纹理
            glUniform1i(glGetUniformLocation(uiShader, "useTexture"), 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, batch.textures[0]);
        }
        else {
            glUniform1i(glGetUniformLocation(uiShader, "useTexture"), 0);
        }

        // 使用索引绘制
        glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT,
            (void*)(batch.indexOffset * sizeof(GLuint)));
    }
}

void AYUIRenderer::endUIFrame() {
    _device->restoreGLState();
}

void AYUIRenderer::flushDirtyElements() {
    for (uint32_t index : _dirtyElements) {
        if (index < _uiElements.size() && _uiElements[index].dirty) {
            updateElementVertices(index);
            _uiElements[index].dirty = false;
        }
    }
    _dirtyElements.clear();

    for (uint32_t index : _dirtyTextElements) {
        if (index < _uiElements.size() && _uiElements[index].dirty) {
            updateTextVertices(index);
            _uiElements[index].dirty = false;
        }
    }
    _dirtyTextElements.clear();
}

void AYUIRenderer::setupUIBuffers()
{
    _uiVAO = _device->createVertexArray();

    _uiVBO = _device->createVertexBuffer(nullptr, 600, "dynamic");

    _uiEBO = _device->createIndexBuffer(nullptr, 600);

    auto stateManager = _device->getGLStateManager();
    _device->saveGLState();
    stateManager->bindVertexArray(_uiVAO);
    stateManager->bindBuffer(GL_ARRAY_BUFFER, _uiVBO);
    stateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _uiEBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, texCoord));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, color));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, normal));

    _device->restoreGLState();
}

void AYUIRenderer::updateElementVertices(uint32_t elementIndex) {
    UIElement& element = _uiElements[elementIndex];

    if (!element.visible) {
        return;
    }

    // 创建新的批次或添加到现有批次
    if (_uiBatches.empty()) {
        UIBatch newBatch;
        _uiBatches.push_back(newBatch);
    }

    UIBatch& currentBatch = _uiBatches.back();

    std::vector<glm::vec3> rectPoints = AYGraphicGenerator::createRectV();
    glm::vec3 normal = AYGraphicGenerator::create2DN();
    std::vector<glm::vec2> texCoords = AYGraphicGenerator::createRectT();
    std::vector<GLuint> indices = AYGraphicGenerator::createRectI(true);

    // 变换顶点位置：缩放和位移
    std::vector<glm::vec3> transformedPositions;
    transformedPositions.reserve(rectPoints.size());
    for (const auto& vertex : rectPoints) {
        // 缩放顶点的x和y分量，z保持不变或设为0
        glm::vec3 scaledVertex = glm::vec3(
            vertex.x * element.size.x,
            vertex.y * element.size.y,
            0.0f // 或者 vertex.z，但在2D中通常为0
        );
        // 平移顶点到element的位置
        glm::vec3 transformedVertex = scaledVertex + element.position;
        transformedPositions.push_back(transformedVertex);
    }

    uint32_t vertexOffset = currentBatch.positions.size();
    uint32_t indexOffset = currentBatch.indices.size();

    for (size_t i = 0; i < transformedPositions.size(); i++) {
        currentBatch.positions.push_back(transformedPositions[i]);
        currentBatch.texCoords.push_back(texCoords[i]);
        currentBatch.colors.push_back(element.color);
        currentBatch.normals.push_back(normal); // 如果UIBatch有normals成员
    }

    for (const auto& index : indices) {
        currentBatch.indices.push_back(indexOffset + index);
    }

    currentBatch.vertexCount += rectPoints.size();
    currentBatch.indexCount += indices.size();

}

void AYUIRenderer::updateTextVertices(uint32_t elementIndex) {
    // 更新文本顶点（需要字体渲染支持）
    UIElement& element = _uiElements[elementIndex];
    if (element.text.empty()) return;

    // 文本渲染逻辑 - 这里需要实际的字体渲染实现
    // 简化处理，暂时跳过具体实现
}

void AYUIRenderer::uploadVertexData() {
    if (_uiBatches.empty()) return;

    // 计算总顶点数和索引数
    size_t totalVertices = 0;
    size_t totalIndices = 0;

    for (const auto& batch : _uiBatches) {
        totalVertices += batch.positions.size();
        totalIndices += batch.indices.size();
    }

    if (totalVertices == 0) return;

    // 准备顶点和索引数据数组
    std::vector<UIVertex> allVertices;
    std::vector<GLuint> allIndices;

    // 为索引计算偏移量
    size_t indexOffset = 0;

    for (const auto& batch : _uiBatches) {
        // 添加顶点数据
        for (int i = 0; i < batch.positions.size(); i++)
        {
            allVertices.push_back({ batch.positions[i], batch.texCoords[i], batch.colors[i], batch.normals[i] });
        }

        // 添加索引数据（需要根据顶点偏移调整索引值）
        for (GLuint index : batch.indices) {
            allIndices.push_back(indexOffset + index);
        }

        indexOffset += batch.positions.size(); // 假设每个位置对应一个顶点
    }

    auto stateManager = _device->getGLStateManager();
    stateManager->bindVertexArray(_uiVAO);
    stateManager->bindBuffer(GL_ARRAY_BUFFER, _uiVBO);

    glBufferData(GL_ARRAY_BUFFER,
        totalVertices * sizeof(UIVertex),
        nullptr,
        GL_DYNAMIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        allVertices.size() * sizeof(UIVertex),
        allVertices.data());

    stateManager->bindBuffer(GL_ARRAY_BUFFER, _uiEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        totalIndices * sizeof(GLuint),
        nullptr,
        GL_DYNAMIC_DRAW);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
        0,
        allIndices.size() * sizeof(GLuint),
        allIndices.data());

    _vertexBufferDirty = false;
}

void AYUIRenderer::_loadUIRendererConfigINI()
{
    _config.loadFromFile(_configPath, AYConfigWrapper::ConfigType::INI);

    _uiShaderName = _config.get<std::string>("shader name.ui", std::string("UIShader"));
    _uiVertexPath = _config.get<std::string>("shader path.ui_vertex",
        AYPath::Engine::getPresetShaderPath() + std::string("UIRenderer/ui.vert"));
    _uiFragmentPath = _config.get<std::string>("shader path.ui_fragment",
        AYPath::Engine::getPresetShaderPath() + std::string("UIRenderer/ui.frag"));
}

void AYUIRenderer::_saveUIRendererConfigINI()
{
    _config.set("shader name.ui", _uiShaderName);
    _config.set("shader path.ui_vertex", _uiVertexPath);
    _config.set("shader path.ui_fragment", _uiFragmentPath);

    _config.saveConfig(_configPath);
}

GLuint AYUIRenderer::_getUIShader(bool reload)
{
    return _device->getShaderV(_uiShaderName, reload, _uiVertexPath, _uiFragmentPath);
}
