#include "AYRenderDevice.h"


bool AYRenderDevice::init(int width, int height)
{
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _window = glfwCreateWindow(width, height, "AliyatRenderer", NULL, NULL);
    if (!_window) return false;

    glfwMakeContextCurrent(_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    glfwSwapInterval(1);

    _stateManager = std::make_unique<AYGLStateManager>();

    _stateManager->setDepthTest(true);
    _stateManager->setBlend(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _shaderManager = std::make_unique<AYShaderManager>();
    return true;
}

void AYRenderDevice::shutdown()
{
}

GLFWwindow* AYRenderDevice::getWindow()
{
    GLFWwindow* win = _window.load();
    if (!win) throw std::runtime_error("Window not available");
    return win;
}

GLuint AYRenderDevice::createVertexBuffer(const void* data, size_t size, const std::string& type)
{
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (type == "dynamic")
    {
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }
    return vbo;
}

GLuint AYRenderDevice::createIndexBuffer(const void* data, size_t size)
{
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return ibo;
}

GLuint AYRenderDevice::createVertexArray()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    return vao;
}

GLuint AYRenderDevice::createTexture2D(const uint8_t* pixels, int width, int height, int channels)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // �����������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = GL_RGBA;
    if (channels == 1) format = GL_RED;
    else if (channels == 2) format = GL_RG;
    else if (channels == 3) format = GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}

GLuint AYRenderDevice::createFontTexture(const uint8_t* pixels, int width, int height)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // �����������������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ʹ�� GL_RED ��ʽ�洢��ͨ�����ݣ�����ͨ��ֻ��Ҫalphaͨ����
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

    return texture;
}

GLuint AYRenderDevice::createShaderProgram(const char* vtx_src, const char* frag_src)
{
    // ����������ɫ��
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vtx_src, NULL);
    glCompileShader(vertexShader);


    // ����Ƭ����ɫ��
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag_src, NULL);
    glCompileShader(fragmentShader);

    // ������ɫ������
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);



    // ɾ����ɫ������
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint AYRenderDevice::getShaderV(const std::string& name, bool reload, const std::string& vertex_shaderPath, const std::string& fragment_shaderPath)
{
    if (reload)
        _shaderManager->reloadShader(name);
    return _shaderManager->loadShader(name, vertex_shaderPath, fragment_shaderPath);
}

void AYRenderDevice::restoreGLState()
{
    // �ָ�����
    _stateManager->useProgram(_previousState.currentProgram);

    // �ָ���Ȳ���
    _stateManager->setDepthTest(_previousState.depthTestEnabled);

    // �ָ����״̬
    _stateManager->setBlend(_previousState.blendEnabled,
        _previousState.currentBlendFunc.src,
        _previousState.currentBlendFunc.dst);

    // �ָ�VAO/VBO
    _stateManager->bindVertexArray(_previousState.currentVAO);
    _stateManager->bindBuffer(GL_ARRAY_BUFFER, _previousState.currentVBO);
    _stateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _previousState.currentEBO);

    // �ָ�����
    for (GLuint unit = 0; unit < _previousState.currentTextureUnits.size(); ++unit) {
        _stateManager->bindTexture(GL_TEXTURE_2D, _previousState.currentTextureUnits[unit], unit);
    }

    // �ָ��ӿ�
    _stateManager->setViewport(_previousState.currentViewport.x,
        _previousState.currentViewport.y,
        _previousState.currentViewport.w,
        _previousState.currentViewport.h);

    // �ָ��߿�
    _stateManager->setLineWidth(_previousState.lineWidth);
}