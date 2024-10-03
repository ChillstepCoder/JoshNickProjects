#pragma once

#include <string>

// These are for checkGLError, move with it when moving to new file
#include <string>
#include <iostream>

#include <GL/glew.h>

namespace Bengine {

    extern void fatalError(std::string errorString);

    // Checks the output of glGetError and prints an appropriate error message if needed.
    // TODO: Put this in a separate file
    inline bool checkGlError(const char* errorLocation) {
        const GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            switch (error) {
            case GL_INVALID_ENUM:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1280: GL_INVALID_ENUM");
                break;
            case GL_INVALID_VALUE:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1281: GL_INVALID_VALUE");
                break;
            case GL_INVALID_OPERATION:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1282: GL_INVALID_OPERATION");
                break;
            case GL_STACK_OVERFLOW:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1283: GL_STACK_OVERFLOW");
                break;
            case GL_STACK_UNDERFLOW:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1284: GL_STACK_UNDERFLOW");
                break;
            case GL_OUT_OF_MEMORY:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1285: GL_OUT_OF_MEMORY");
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1285: GL_INVALID_FRAMEBUFFER_OPERATION");
                break;
            default:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code " + std::to_string(error) + ": UNKNOWN");
                break;
            }
            __debugbreak(); // This automatically triggers a breakpoint if the debugger is attached
            return true;
        }
        return false;
    }

}