#pragma once
#include "GLFW/glfw3.h"
#include "glad/glad.h"

class AYGLStateTracker 
{
public:
    // ���캯����ʼ��Ĭ��״̬
    AYGLStateTracker() {
        reset();
    }

    // ��������״̬ΪĬ��
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
        currentTextureUnits.resize(8, 0); // ����֧��8������Ԫ
        currentFBO = 0;
        currentViewport = { 0, 0, 0, 0 };
        lineWidth = 1.0f;
        // ������Ӹ�����Ҫ���ٵ�״̬
    }

    // ״̬����
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