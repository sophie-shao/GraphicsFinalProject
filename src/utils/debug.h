#pragma once
#include <GL/glew.h>
#include <iostream>

namespace Debug {
inline void glErrorCheck(const char* file, int line) {
    GLenum error;
    bool hasError = false;
    while ((error = glGetError()) != GL_NO_ERROR) {
        hasError = true;
        std::string errorStr;
        switch (error) {
        case GL_INVALID_ENUM: errorStr = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE: errorStr = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: errorStr = "GL_INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY: errorStr = "GL_OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        default: errorStr = "Unknown GL error"; break;
        }
        std::cerr << "[OpenGL Error] " << errorStr
                  << " at " << file << ":" << line << std::endl;
    }
    if (!hasError) {
        //maybe later
    }
}
}

#define glErrorCheck() Debug::glErrorCheck(__FILE__, __LINE__)
