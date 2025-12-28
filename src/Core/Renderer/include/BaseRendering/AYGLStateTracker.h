#pragma once
#include "GLFW/glfw3.h"
#include "glad/glad.h"
namespace ayt::engine::render
{
    class AYGLStateTracker
    {
    public:
        // 构造函数初始化默认状态
        AYGLStateTracker() {
            reset();
        }

        // 重置所有状态为默认
        void reset() {
            currentProgram = 0;
            depthTestEnabled = false;
            depthMaskEnabled = true;
            cullFaceEnabled = false;
            blendEnabled = false;
            currentBlendFunc = { GL_ONE, GL_ZERO };
            currentDepthFunc = GL_LESS;
            currentVAO = 0;
            currentVBO = 0;
            currentEBO = 0;
            currentTextureUnits.clear();
            currentTextureUnits.resize(8, 0); // 假设支持8个纹理单元
            currentFBO = 0;
            currentViewport = { 0, 0, 0, 0 };
            lineWidth = 1.0f;
            // 可以添加更多需要跟踪的状态
        }

        // 状态变量
        GLuint currentProgram;
        bool depthTestEnabled;
        bool depthMaskEnabled;
        bool cullFaceEnabled;
        bool blendEnabled;
        struct BlendFunc { GLenum src, dst; } currentBlendFunc;
        GLenum currentDepthFunc;
        GLuint currentVAO;
        GLuint currentVBO;
        GLuint currentEBO;
        std::vector<GLuint> currentTextureUnits;
        GLuint currentFBO;
        struct Viewport { GLint x, y; GLsizei w, h; } currentViewport;
        float lineWidth;
    };
}