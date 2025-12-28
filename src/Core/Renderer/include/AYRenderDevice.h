#pragma once
#include "ECRendererDependence.h"
#include "AYGLStateManager.h"
#include "AYShaderManager.h"
#include "AYTextureManager.h"
#include <Windows.h>
#include <mutex>
namespace ayt::engine::render
{
    class AYRenderDevice {
    public:
        AYRenderDevice();
        ~AYRenderDevice();
        bool init(int width, int height);
        void shutdown();

    public:
        // 窗口样式设置
        using ViewportCallback = std::function<void(int, int)>;

        GLFWwindow* getWindow();
        // 透明度与颜色键互斥，当启用透明度时颜色键将失效
        void setWindowDesktopEffect(float opacity = 1.f, bool clickThrough = false, COLORREF colorkey = RGB(0, 0, 0));
        void setWindowAlwaysOnTop(bool topmost);
        void OnWindowSizeChanged(int width, int height);
        void setViewportCallback(ViewportCallback callback);
        void removeViewportCallback();

    public:
        // GL base
        GLuint createVertexBuffer(const void* data, size_t size, const std::string& type = "static");// type: "dynamic" or "static", 只传入数据
        GLuint createIndexBuffer(const void* data, size_t size);
        GLuint createVertexArray();

        bgfx::VertexBufferHandle createVertexBufferB(const bgfx::VertexLayout& layout, const void* data, size_t size, const std::string& type = "static");
        bgfx::IndexBufferHandle createIndexBufferB(const void* data, size_t size);
        bgfx::VertexLayout createVertexLayoutB();    //仅作示例，不要使用

        const bgfx::Memory* createMemoryB(const void* data, size_t size);
    public:
        // 纹理
        enum class TextureType {
            Standard,    // 标准纹理 (RGBA/RGB等)
            Font,        // 字体纹理 (单通道)
            Video,       // 视频纹理 (动态更新)
            Empty        // 空纹理 (仅分配空间)
        };

        struct TextureParams {
            uint64_t flags = 0;
            GLenum wrapS = GL_REPEAT;
            GLenum wrapT = GL_REPEAT;
            GLenum minFilter = GL_LINEAR;
            GLenum magFilter = GL_LINEAR;
            bool generateMipmap = false;
        };

        AYTextureManager* getTextureManager() { return _textureManager.get(); }

        TextureParams getDefaultTextureParams(TextureType type);
        TextureParams getDefaultTextureParamsB(TextureType type);

        // 纹理通用接口
        GLuint createTexture(TextureType type, const uint8_t* pixels,
            int width, int height, int channels = 4,
            const TextureParams* customParams = nullptr);

        bgfx::TextureHandle createTextureB(
            TextureType type,
            const uint8_t* pixels,
            int width,
            int height,
            int channels,
            const TextureParams* customParams = nullptr);

        void updateTexture(GLuint textureID,
            const uint8_t* pixels,
            int width,
            int height,
            GLenum format = GL_RGBA);

        void updateTextureB(bgfx::TextureHandle texture,
            const uint8_t* pixels,
            int width,
            int height,
            bgfx::TextureFormat::Enum format);

        GLuint createTexture2D(const uint8_t* pixels, int width, int height, int channels = 4);
        GLuint createFontTexture(const uint8_t* pixels, int width, int height);
        GLuint createVideoTexture(int width, int height);

        bgfx::TextureHandle createTexture2DB(const uint8_t* pixels, int width, int height, int channels = 4);
        bgfx::TextureHandle createFontTextureB(const uint8_t* pixels, int width, int height);
        bgfx::TextureHandle createVideoTextureB(int width, int height);

    public:
        // Shader
        GLuint createShaderProgram(const char* vtx_src, const char* frag_src);
        GLuint getShaderV(const std::string& name,
            bool reload = false,
            const std::string& vertex_shaderPath = "",
            const std::string& fragment_shaderPath = "");

        bgfx::ProgramHandle createShaderProgramB(const char* vtx_name,
            const char* vtx_src,
            const char* frag_name,
            const char* frag_src);
        bgfx::ProgramHandle getShaderB(const std::string& name,
            bool reload = false,
            const std::string& vertex_shaderPath = "",
            const std::string& fragment_shaderPath = "");

        AYGLStateManager* getGLStateManager() { return _stateManager.get(); }
        void saveGLState() { _previousState = _stateManager->getState(); }
        void restoreGLState();


    private:
        // windows
        static void _viewportCallbackWrapper(GLFWwindow* window, int width, int height);
        std::atomic<GLFWwindow*> _window{ nullptr };
        ViewportCallback _callback;
        std::mutex _callbackMutex;
        void _loadDeviceWindowConfigINI();
        void _saveDeviceWindowConfigINI();

        config::Configer _config;
        std::string _configPath;
        std::string _renderBackend;
        bool _isEnableWindowEffect;
        bool _isShowBorder;
        bool _isAlwaysOnTop;
        bool _isMousePenetrate;
        float _wOpacity;
        int _colorKeyR, _colorKeyG, _colorKeyB;


        // gl states
        std::unique_ptr<AYGLStateManager> _stateManager;
        std::unique_ptr<AYShaderManager> _shaderManager;
        std::unique_ptr<AYTextureManager> _textureManager;

        AYGLStateTracker _previousState;

    };
}