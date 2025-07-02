#pragma once
#include "ECRendererDependence.h"
#include "AYGLStateManager.h"
#include "AYShaderManager.h"

class AYRenderDevice {
public:
    bool init(int width, int height);
    void shutdown();

    GLFWwindow* getWindow();

    // type: "dynamic" or "static"
    GLuint createVertexBuffer(const void* data, size_t size, const std::string& type = "static");
    GLuint createIndexBuffer(const void* data, size_t size);
    GLuint createVertexArray();
    GLuint createTexture2D(const uint8_t* pixels, int width, int height, int channels = 4);
    GLuint createFontTexture(const uint8_t* pixels, int width, int height);
    GLuint createShaderProgram(const char* vtx_src, const char* frag_src);
    GLuint getShaderV(const std::string& name,
        bool reload = false,
        const std::string& vertex_shaderPath = "",
        const std::string& fragment_shaderPath = "");

    AYGLStateManager* getGLStateManager() { return _stateManager.get(); }
    void saveGLState() { _previousState = _stateManager->getState(); }
    void restoreGLState();
private:
    std::atomic<GLFWwindow*> _window {nullptr};
    std::unique_ptr<AYGLStateManager> _stateManager;
    std::unique_ptr<AYShaderManager> _shaderManager;
    AYGLStateTracker _previousState;
};
