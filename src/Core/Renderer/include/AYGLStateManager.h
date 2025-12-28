#pragma once
#include "BaseRendering/AYGLStateTracker.h"
namespace ayt::engine::render
{
    class AYGLStateManager {
    public:
        AYGLStateManager() {
            // 初始化时获取驱动默认状态
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
            state.currentTextureUnits.resize(maxTextureUnits, 0);
        }

        // 使用着色器程序
        void useProgram(GLuint program) {
            if (state.currentProgram != program) {
                glUseProgram(program);
                state.currentProgram = program;
            }
        }

        // 深度测试
        void setDepthTest(bool enable) {
            if (state.depthTestEnabled != enable) {
                enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
                state.depthTestEnabled = enable;
            }
        }

        // 写入深度缓冲
        void setDepthMask(bool enable)
        {
            if (state.depthMaskEnabled != enable) {
                enable ? glDepthMask(GL_TRUE) : glDepthMask(GL_FALSE);
                state.depthMaskEnabled = enable;
            }
        }

        void setDepthFunc(GLenum method)
        {
            if (state.currentDepthFunc != method)
            {
                glDepthFunc(method);
                state.currentDepthFunc == method;
            }
        }

        // 背面剔除
        void setCullFace(bool enable)
        {
            if (state.cullFaceEnabled != enable) {
                enable ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
                state.cullFaceEnabled = enable;
            }
        }

        // 混合设置
        void setBlend(bool enable, GLenum srcFactor = GL_ONE, GLenum dstFactor = GL_ZERO) {
            if (enable != state.blendEnabled) {
                enable ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
                state.blendEnabled = enable;
            }

            if (enable && (srcFactor != state.currentBlendFunc.src ||
                dstFactor != state.currentBlendFunc.dst)) {
                glBlendFunc(srcFactor, dstFactor);
                state.currentBlendFunc = { srcFactor, dstFactor };
            }
        }

        // 绑定VAO
        void bindVertexArray(GLuint vao) {
            if (state.currentVAO != vao) {
                glBindVertexArray(vao);
                state.currentVAO = vao;
            }
        }

        // 绑定VBO
        void bindBuffer(GLenum target, GLuint buffer) {
            switch (target) {
            case GL_ARRAY_BUFFER:
                if (state.currentVBO != buffer) {
                    glBindBuffer(target, buffer);
                    state.currentVBO = buffer;
                }
                break;
            case GL_ELEMENT_ARRAY_BUFFER:
                if (state.currentEBO != buffer) {
                    glBindBuffer(target, buffer);
                    state.currentEBO = buffer;
                }
                break;
            default:
                glBindBuffer(target, buffer);
            }
        }

        // 绑定纹理
        void bindTexture(GLenum target, GLuint texture, GLuint unit = 0) {
            if (unit >= state.currentTextureUnits.size()) {
                // 处理不支持的纹理单元
                return;
            }

            if (state.currentTextureUnits[unit] != texture) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(target, texture);
                state.currentTextureUnits[unit] = texture;
            }
        }

        // 设置视口
        void setViewport(GLint x, GLint y, GLsizei w, GLsizei h) {
            if (x != state.currentViewport.x || y != state.currentViewport.y ||
                w != state.currentViewport.w || h != state.currentViewport.h) {
                glViewport(x, y, w, h);
                state.currentViewport = { x, y, w, h };
            }
        }

        // 设置线宽
        void setLineWidth(float width) {
            if (state.lineWidth != width) {
                glLineWidth(width);
                state.lineWidth = width;
            }
        }

        // 获取当前状态(用于调试)
        const AYGLStateTracker& getState() const { return state; }

    private:
        AYGLStateTracker state;
        GLint maxTextureUnits;
    };
}