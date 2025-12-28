#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <GLFW/glfw3.h>
#include <bx/math.h>
#include <cmath>
#include "BaseRendering/AYBgfxCreator.h"
// 获取原生窗口句柄
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#    define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_OSX
#    define GLFW_EXPOSE_NATIVE_COCOA
#elif BX_PLATFORM_WINDOWS
#    define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

// 定义顶点数据结构（适配你的Shader输入）
struct PosTexColorVertex {
    float x, y, z;      // a_position
    float u, v;         // a_texcoord0
    uint32_t abgr;      // a_color0

    static void init() {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)  // a_position
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float) // a_texcoord0
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true) // a_color0
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosTexColorVertex::ms_layout;

int main() {
    // 初始化 GLFW
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "bgfx UI渲染示例", nullptr, nullptr);

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

    // 设置清屏颜色
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

    // 初始化顶点布局
    PosTexColorVertex::init();

    // 创建顶点缓冲区（一个简单的四边形，带纹理坐标和颜色）
    PosTexColorVertex vertices[] = {
        // x    y    z    u    v       color
        {-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0xFFFFFFFF}, // 左上
        { 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF}, // 右上
        {-0.5f,-0.5f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFF}, // 左下
        { 0.5f,-0.5f, 0.0f, 1.0f, 1.0f, 0xFFFFFFFF},  // 右下
    };
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices, sizeof(vertices)),
        PosTexColorVertex::ms_layout
    );

    // 创建索引缓冲区
    uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
        bgfx::makeRef(indices, sizeof(indices))
    );

    // 创建纹理采样器Uniform（适配你的Shader中的s_texColor）
    bgfx::UniformHandle s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    bgfx::UniformHandle u_uiParams = bgfx::createUniform("u_uiParams", bgfx::UniformType::Vec4);
    bgfx::UniformHandle u_color = bgfx::createUniform("u_uiElementColor", bgfx::UniformType::Vec4);
    bgfx::UniformHandle u_cornerParams = bgfx::createUniform("u_cornerParams", bgfx::UniformType::Vec4);
    bgfx::UniformHandle u_borderColor = bgfx::createUniform("u_borderColor", bgfx::UniformType::Vec4);
    bgfx::UniformHandle u_borderParams = bgfx::createUniform("u_borderParams", bgfx::UniformType::Vec4);

    // 初始化AYBgfxCreator并加载Shader
    AYBgfxCreator creator;
    creator.getProgram("vs_ui.sc|fs_ui.sc");
    bgfx::TextureInfo info;
    bgfx::TextureHandle texture = creator.loadTexture(
        "assets/core/textures/icon_64_64.png",
        BGFX_TEXTURE_NONE,
        0,          // skip
        &info       // 获取纹理信息
    );

    // 现在可以访问纹理信息
    printf("Texture size: %dx%d, format: %d\n", info.width, info.height, info.format);

    bgfx::ProgramHandle program = creator.getProgram("vs_ui.sc|fs_ui.sc");
    // 主循环
    float time = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // 设置视图和投影矩阵（2D正交投影）
        float view[16];
        bx::mtxIdentity(view);
        float ortho[16];
        bx::mtxOrtho(ortho, 0.0f, 800.0f, 600.0f, 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view, ortho);

        // 设置模型矩阵（简单动画）
        float temp[16];
        float model[16];
        bx::mtxIdentity(temp);
        bx::mtxIdentity(model);
        bx::mtxScale(temp, 100.0f, 40.0f, 1.0f);
        bx::mtxMul(model, model, temp);
        //bx::mtxRotateZ(temp, time * 0.2f);
        //bx::mtxMul(model, model, temp);
        bx::mtxTranslate(temp, 400.f, 300.f, 0);
        bx::mtxMul(model, model, temp);

        // 设置视图矩形
        bgfx::setViewRect(0, 0, 0, 800, 600);

        // 绑定纹理
        //bgfx::setTexture(0, s_texColor, texture);

        // 提交绘制命令
        bgfx::setTransform(model);

        bgfx::setUniform(u_uiParams, new float[4] {1, time, 1, 1});
        bgfx::setUniform(u_color, new float[4] {0.5f, 0.5f, 0.5f,1});
        bgfx::setUniform(u_cornerParams, new float[4] {0.0f, 0.5, 0.005f, 0});
        bgfx::setUniform(u_borderColor, new float[4] {0, 0, 0, 1});
        bgfx::setUniform(u_borderParams, new float[4] {0.035f, 0.005f, 1, 0});

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);
        bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ALPHA);
        bgfx::submit(0, program);

        // 渲染帧
        bgfx::frame();

        // 更新时间
        time += 0.01f;
    }

    // 清理资源
    bgfx::destroy(texture);
    bgfx::destroy(s_texColor);
    bgfx::destroy(vbh);
    bgfx::destroy(ibh);
    bgfx::shutdown();
    glfwTerminate();
    return 0;
}