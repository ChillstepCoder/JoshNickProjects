// Window.cpp

#include "Window.h"
#include "JAGErrors.h"
#include <iostream>
#include <sstream> // For std::stringstream

#include <GL/glew.h>
#include <GL/gl.h>


namespace JAGEngine {
  Window::Window() {
  }

  Window::~Window() {
    if (_glContext != nullptr) {
      SDL_GL_DeleteContext(_glContext);
    }
    if (_sdlWindow != nullptr) {
      SDL_DestroyWindow(_sdlWindow);
    }
  }

  int Window::create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFlags) {
    Uint32 flags = SDL_WINDOW_OPENGL;
    if (currentFlags & static_cast<unsigned int>(windowFlags::INVISIBLE)) {
      flags |= SDL_WINDOW_HIDDEN;
    }
    if (currentFlags & static_cast<unsigned int>(windowFlags::FULLSCREEN)) {
      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    if (currentFlags & static_cast<unsigned int>(windowFlags::BORDERLESS)) {
      flags |= SDL_WINDOW_BORDERLESS;
    }

    _sdlWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, flags);
    if (_sdlWindow == nullptr) {
      fatalError("SDL Window could not be created!");
    }

    _glContext = SDL_GL_CreateContext(_sdlWindow);
    if (_glContext == nullptr) {
      fatalError("SDL_GL context could not be created.");
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (error != GLEW_OK) {
      fatalError("Could not initialize GLEW.");
    }

    // Enable OpenGL debug output
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(Window::glDebugOutput, nullptr);

    // Check OpenGL version
    std::printf("***   OpenGL Version: %s   ***\n", glGetString(GL_VERSION));

    // Set VSync
    SDL_GL_SetSwapInterval(1);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.85f, 1.0f);

    return 0;
  }


  void Window::swapBuffer() {
    SDL_GL_SwapWindow(_sdlWindow);
  }

  // OpenGL debugging callback function
  void Window::glDebugOutput(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam) {
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131186)
      return;

    std::stringstream ss;

    ss << "---------------" << std::endl;
    ss << "Debug message (" << id << " - 0x" << std::hex << id << std::dec << "): " << message << std::endl;

    switch (source) {
    case GL_DEBUG_SOURCE_API:             ss << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ss << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: ss << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     ss << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     ss << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           ss << "Source: Other"; break;
    }
    ss << std::endl;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               ss << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ss << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         ss << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         ss << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              ss << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          ss << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           ss << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               ss << "Type: Other"; break;
    }
    ss << std::endl;

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         ss << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       ss << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          ss << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: ss << "Severity: notification"; break;
    }
    ss << std::endl;
    ss << std::endl;

    std::cout << ss.str();
    // Uncomment the line below if you want to trigger a breakpoint when a debug message is received
    // __debugbreak();
  }
}
