//Window.cpp

#include "Window.h"
#include "Errors.h"

namespace JAGEngine {
  Window::Window() {
  }

  Window::~Window() {
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

    SDL_GLContext glContext = SDL_GL_CreateContext(_sdlWindow);
    if (glContext == nullptr) {
      fatalError("SDL_GL context could not be created.");
    }

    GLenum error = glewInit();
    if (error != GLEW_OK) {
      fatalError("Could not initialize glew.");
    }

    //check opengl version
    std::printf("***   OpenGL Version: %s   ***", glGetString(GL_VERSION));

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    //set vsync
    SDL_GL_SetSwapInterval(1);

    return 0;
  }

  void Window::swapBuffer() {
    SDL_GL_SwapWindow(_sdlWindow);
  }
}
