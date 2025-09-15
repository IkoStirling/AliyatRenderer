#pragma once
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bimg/bimg.h>
#include <bgfx/platform.h>
#include "AYConfigWrapper.h"
#include "AYPath.h"
#include <iostream>
#include <sstream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <atomic>
#include <functional>
#include <spdlog/spdlog.h>


#define AY_CHECK_GL_ERROR(context) \
    do { \
        GLenum err; \
        const char* errContext = (context); \
        while ((err = glGetError()) != GL_NO_ERROR) { \
            const char* errStr = ""; \
            switch(err) { \
                case GL_INVALID_ENUM:      errStr = "GL_INVALID_ENUM"; break; \
                case GL_INVALID_VALUE:     errStr = "GL_INVALID_VALUE"; break; \
                case GL_INVALID_OPERATION: errStr = "GL_INVALID_OPERATION"; break; \
                case GL_OUT_OF_MEMORY:    errStr = "GL_OUT_OF_MEMORY"; break; \
                case GL_INVALID_FRAMEBUFFER_OPERATION: errStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break; \
                default:                   errStr = "UNKNOWN_ERROR"; break; \
            } \
            spdlog::debug("[OpenGL Error] {} (0x{:x}) in {} at {}:{}", \
                errStr, err, errContext, __FILE__, __LINE__); \
            if (err == GL_OUT_OF_MEMORY) { \
                spdlog::error("[AY_CHECK_GL_ERROR] Fatal OpenGL error: Out of memory"); \
                std::terminate(); \
            } \
        } \
    } while (0)
