#include "AYRenderDevice.h"
#include "AYRendererManager.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

AYRenderDevice::AYRenderDevice() :
    _configPath(AYPath::Engine::getPresetConfigPath() + std::string("Renderer/RenderDevice/config.ini"))
{
    _loadDeviceWindowConfigINI();
}

AYRenderDevice::~AYRenderDevice()
{
    _saveDeviceWindowConfigINI();
}

bool AYRenderDevice::init(int width, int height)
{
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (!_isShowBorder)
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); 

    _window = glfwCreateWindow(width, height, "AliyatRenderer", NULL, NULL);
    if (!_window) return false;
    glfwSetFramebufferSizeCallback(_window, &AYRenderDevice::_viewportCallbackWrapper); 

    glfwMakeContextCurrent(_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    setWindowAlwaysOnTop(_isAlwaysOnTop);
    setWindowDesktopEffect(_wOpacity, _isMousePenetrate, RGB(_colorKeyR, _colorKeyG, _colorKeyB));

    glfwSwapInterval(1);

    _stateManager = std::make_unique<AYGLStateManager>();

    _stateManager->setDepthTest(true);
    _stateManager->setBlend(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _shaderManager = std::make_unique<AYShaderManager>();
    return true;
}

void AYRenderDevice::shutdown()
{
    removeViewportCallback();
}

GLFWwindow* AYRenderDevice::getWindow()
{
    GLFWwindow* win = _window.load();
    if (!win) throw std::runtime_error("Window not available");
    return win;
}

void AYRenderDevice::setWindowDesktopEffect(float opacity, bool clickThrough, COLORREF colorkey)
{
    HWND hwnd = glfwGetWin32Window(_window.load());
    if (!hwnd) {
        std::cerr << "Failed to get Win32 window handle!" << std::endl;
        return;
    }

    if (opacity != _wOpacity) _wOpacity = opacity;
    if (clickThrough != _isMousePenetrate) _isMousePenetrate = clickThrough;
    if (colorkey != RGB(_colorKeyR, _colorKeyG, _colorKeyB))
    {
        _colorKeyR = GetRValue(colorkey);
        _colorKeyG = GetGValue(colorkey);
        _colorKeyB = GetBValue(colorkey);
    }

    // 设置分层窗口
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);

    
    if (opacity >= 0.99f)
    {
        //设置颜色键控（将黑色设为透明）
        SetLayeredWindowAttributes(hwnd, colorkey, 0, LWA_COLORKEY);
    }
    else
    {
        // 设置透明度 (0-255)
        BYTE alpha = static_cast<BYTE>(opacity * 255);
        SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
    }

    // 设置鼠标穿透穿透
    if (clickThrough) {
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    }
    else {
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }
}

void AYRenderDevice::OnWindowSizeChanged(int width, int height)
{
    // 像素级透明度控制，因复杂度较高废弃
}

void AYRenderDevice::setWindowAlwaysOnTop(bool topmost)
{
    HWND hwnd = glfwGetWin32Window(_window.load());
    if (!hwnd) {
        std::cerr << "Failed to get Win32 window handle!" << std::endl;
        return;
    }

    if (topmost != _isAlwaysOnTop) _isAlwaysOnTop = topmost;

    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (topmost) {
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TOPMOST);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else {
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TOPMOST);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void AYRenderDevice::setViewportCallback(ViewportCallback callback)
{
    std::lock_guard<std::mutex> lock(_callbackMutex);
    _callback = std::move(callback);
}

void AYRenderDevice::removeViewportCallback()
{
    std::lock_guard<std::mutex> lock(_callbackMutex);
    if (_window) {
        glfwSetFramebufferSizeCallback(_window, nullptr);
    }
    _callback = nullptr;
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

    // 设置纹理参数
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

    // 字体纹理的特殊设置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 使用 GL_RED 格式存储单通道数据（字体通常只需要alpha通道）
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

    return texture;
}

GLuint AYRenderDevice::createShaderProgram(const char* vtx_src, const char* frag_src)
{
    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vtx_src, NULL);
    glCompileShader(vertexShader);


    // 创建片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag_src, NULL);
    glCompileShader(fragmentShader);

    // 创建着色器程序
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);



    // 删除着色器对象
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
    // 恢复程序
    _stateManager->useProgram(_previousState.currentProgram);

    // 恢复深度测试
    _stateManager->setDepthTest(_previousState.depthTestEnabled);

    // 恢复深度缓冲写入
    _stateManager->setDepthMask(_previousState.depthMaskEnabled);

    // 恢复背面剔除
    _stateManager->setCullFace(_previousState.cullFaceEnabled);

    // 恢复混合状态
    _stateManager->setBlend(_previousState.blendEnabled,
        _previousState.currentBlendFunc.src,
        _previousState.currentBlendFunc.dst);

    // 恢复VAO/VBO
    _stateManager->bindVertexArray(_previousState.currentVAO);
    _stateManager->bindBuffer(GL_ARRAY_BUFFER, _previousState.currentVBO);
    _stateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _previousState.currentEBO);

    // 恢复纹理
    for (GLuint unit = 0; unit < _previousState.currentTextureUnits.size(); ++unit) {
        _stateManager->bindTexture(GL_TEXTURE_2D, _previousState.currentTextureUnits[unit], unit);
    }

    // 恢复视口
    _stateManager->setViewport(_previousState.currentViewport.x,
        _previousState.currentViewport.y,
        _previousState.currentViewport.w,
        _previousState.currentViewport.h);

    // 恢复线宽
    _stateManager->setLineWidth(_previousState.lineWidth);
}

void AYRenderDevice::_viewportCallbackWrapper(GLFWwindow* window, int width, int height) {
    auto* device = GET_CAST_MODULE(Mod_Renderer, "Renderer")->getRenderDevice();
    if (!device) return;

    std::function<void(int, int)> callback;
    {
        std::lock_guard<std::mutex> lock(device->_callbackMutex);
        callback = device->_callback; 
    }

    if (device->_callback) {
        device->_callback(width, height);
    }
}

void AYRenderDevice::_loadDeviceWindowConfigINI()
{
    _config.loadFromFile(_configPath, AYConfigWrapper::ConfigType::INI);

    _isShowBorder = _config.get<bool>("window config.show_border", true);
    _isAlwaysOnTop = _config.get<bool>("window config.always_on_top", false);
    _isMousePenetrate = _config.get<bool>("window config.mouse_penetrate", false);
    _wOpacity = _config.get<float>("window config.opacity", 1.f);
    _colorKeyR = _config.get<uint32_t>("window config.color_key_r", 1); //默认错开纯黑的颜色键效果
    _colorKeyG = _config.get<uint32_t>("window config.color_key_g", 0);
    _colorKeyB = _config.get<uint32_t>("window config.color_key_b", 0);
}

void AYRenderDevice::_saveDeviceWindowConfigINI()
{
    _config.set<bool>("window config.show_border", _isShowBorder);
    _config.set<bool>("window config.always_on_top", _isAlwaysOnTop);
    _config.set<bool>("window config.mouse_penetrate", _isMousePenetrate);
    _config.set<float>("window config.opacity", _wOpacity);
    _config.set<uint32_t>("window config.color_key_r", _colorKeyR);
    _config.set<uint32_t>("window config.color_key_g", _colorKeyG);
    _config.set<uint32_t>("window config.color_key_b", _colorKeyB);

    _config.saveConfig(_configPath);
}