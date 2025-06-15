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
    //glfwSetKeyCallback(windowInstance, keyCallBack);
    //glfwSetMouseButtonCallback(windowInstance, mouseButtonCallback);
    //glfwSetCursorPosCallback(windowInstance, cursorPosCallback);
    //glfwSetScrollCallback(windowInstance, scrollCallback);
    //glfwSetWindowUserPointer(windowInstance, this);

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
    return _window;
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
	return GLuint();
}

GLuint AYRenderDevice::createVertexArray()
{
	return GLuint();
}

GLuint AYRenderDevice::createTexture2D(const uint8_t* pixels, int width, int height)
{
	return GLuint();
}
