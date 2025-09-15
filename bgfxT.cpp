#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <GLFW/glfw3.h>
#include <cmath>  // 用于 sinf 函数

// 获取原生窗口句柄（Windows/Linux/macOS）
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#    define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_OSX
#    define GLFW_EXPOSE_NATIVE_COCOA
#elif BX_PLATFORM_WINDOWS
#    define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

int main() {
    // 初始化 GLFW
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "bgfx动态示例", nullptr, nullptr);

    // 初始化 bgfx
    bgfx::PlatformData platformData;
#if BX_PLATFORM_WINDOWS
    platformData.nwh = glfwGetWin32Window(window);
#endif
    bgfx::Init init;
    init.platformData = platformData;
    init.resolution.width = 800;
    init.resolution.height = 600;
    init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx::init(init);

    // 主循环
    float time = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // 动态计算颜色（RGB 渐变）
        time += 0.01f;
        uint8_t r = static_cast<uint8_t>(sinf(time) * 127 + 128);
        uint8_t g = static_cast<uint8_t>(sinf(time + 2.0f) * 127 + 128);
        uint8_t b = static_cast<uint8_t>(sinf(time + 4.0f) * 127 + 128);
        uint32_t clearColor = (r << 24) | (g << 16) | (b << 8) | 0xFF;  // RGBA

        // 设置动态清屏颜色
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor);

        // 渲染
        bgfx::setViewRect(0, 0, 0, 800, 600);
        bgfx::touch(0);
        bgfx::frame();
    }

    // 清理
    bgfx::shutdown();
    glfwTerminate();
    return 0;
}