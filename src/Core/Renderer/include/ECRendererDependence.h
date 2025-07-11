#pragma once
#include <iostream>
#include <sstream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <atomic>
#include <functional>

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
            std::cerr << "[OpenGL Error] " << errStr << " (0x" << std::hex << err << ") " \
                      << "in " << errContext << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            if (err == GL_OUT_OF_MEMORY) { \
                std::cerr << "Fatal OpenGL error: Out of memory" << std::endl; \
                std::terminate(); \
            } \
        } \
    } while (0)
