// Window.h

#pragma once

#include <string>
#include <SDL/SDL.h>
#include <GL/glew.h> // Include GLEW before OpenGL headers
#include <GL/gl.h>   // Include OpenGL headers


namespace JAGEngine {
  enum class windowFlags { INVISIBLE = 0x1, FULLSCREEN = 0x2, BORDERLESS = 0x4 };

  class Window {
  public:
    Window();
    ~Window();

    int create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFlags);
    void swapBuffer();

    SDL_Window* getSDLWindow() const { return _sdlWindow; }
    SDL_GLContext getGLContext() const { return _glContext; }

    static void glDebugOutput(GLenum source,
      GLenum type,
      GLuint id,
      GLenum severity,
      GLsizei length,
      const GLchar* message,
      const void* userParam);

    // Getters
    int getScreenWidth() { return m_screenWidth; }
    int getScreenHeight() { return m_screenHeight; }

  private:
    SDL_Window* _sdlWindow = nullptr;
    SDL_GLContext _glContext = nullptr;

    int m_screenWidth = 1920;
    int m_screenHeight = 1080;

  };
}
