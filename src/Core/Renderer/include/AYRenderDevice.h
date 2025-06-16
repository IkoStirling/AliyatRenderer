#pragma once
#include "ECRendererDependence.h"


class AYRenderDevice {
public:
    bool init(int width, int height);
    void shutdown();

    GLFWwindow* getWindow();

    GLuint createVertexBuffer(const void* data, size_t size);
    GLuint createIndexBuffer(const void* data, size_t size);
    GLuint createVertexArray();
    GLuint createTexture2D(const uint8_t* pixels, int width, int height, int channels = 4);
    GLuint createFontTexture(const uint8_t* pixels, int width, int height);
    GLuint createShaderProgram(const char* vtx_src, const char* frag_src);

private:
    std::atomic<GLFWwindow*> _window {nullptr};

    
public: // 按键回调接口
    using KeyCallback = std::function<void(int, int, int, int)>;
    using MouseCallback = std::function<void(double, double)>;
    using MouseButtonCallback = std::function<void(int, int, int)>;
    using ScrollCallback = std::function<void(double, double)>;

    void setKeyCallback(const KeyCallback& callback);
    void setMouseCallback(const MouseCallback& callback);
    void setMouseButtonCallback(const MouseButtonCallback& callback);
    void setScrollCallback(const ScrollCallback& callback);

private:
    // 回调函数存储
    KeyCallback _keyCallback;
    MouseCallback _mouseCallback;
    MouseButtonCallback _mouseButtonCallback;
    ScrollCallback _scrollCallback;

    // 静态回调函数(转发给实例)
    static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset);
};
