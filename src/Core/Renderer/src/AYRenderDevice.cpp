#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
#include "AYRenderDevice.h"
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

    //glfwSetFramebufferSizeCallback(windowInstance, framebufferSizeCallback);
    glfwSetKeyCallback(_window, keyCallbackWrapper);
    glfwSetMouseButtonCallback(_window, mouseButtonCallbackWrapper);
    glfwSetCursorPosCallback(_window, mouseCallbackWrapper);
    glfwSetScrollCallback(_window, scrollCallbackWrapper);
    glfwSetWindowUserPointer(_window, this);

    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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



GLuint AYRenderDevice::createVertexBuffer(const void* data, size_t size)
{
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
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

void AYRenderDevice::setKeyCallback(const KeyCallback& callback)
{
    _keyCallback = callback;
}

void AYRenderDevice::setMouseCallback(const MouseCallback& callback)
{
    _mouseCallback = callback;
}

void AYRenderDevice::setMouseButtonCallback(const MouseButtonCallback& callback)
{
    _mouseButtonCallback = callback;
}

void AYRenderDevice::setScrollCallback(const ScrollCallback& callback)
{
    _scrollCallback = callback;
}

void AYRenderDevice::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto device = static_cast<AYRenderDevice*>(glfwGetWindowUserPointer(window));
    if (device && device->_keyCallback) {
        device->_keyCallback(key, scancode, action, mods);
    }
}

void AYRenderDevice::mouseCallbackWrapper(GLFWwindow* window, double xpos, double ypos)
{
    auto device = static_cast<AYRenderDevice*>(glfwGetWindowUserPointer(window));
    if (device && device->_mouseCallback) {
        device->_mouseCallback(xpos, ypos);
    }
}

void AYRenderDevice::mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods)
{
    auto device = static_cast<AYRenderDevice*>(glfwGetWindowUserPointer(window));
    if (device && device->_mouseButtonCallback) {
        device->_mouseButtonCallback(button, action, mods);
    }
}

void AYRenderDevice::scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
    auto device = static_cast<AYRenderDevice*>(glfwGetWindowUserPointer(window));
    if (device && device->_scrollCallback) {
        device->_scrollCallback(xoffset, yoffset);
    }
}
