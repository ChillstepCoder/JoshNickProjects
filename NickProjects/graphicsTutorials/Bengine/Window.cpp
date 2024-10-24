#include "Window.h"
#include  "BengineErrors.h"

namespace Bengine {

    Window::Window() {

    }
    Window::~Window() {

    }

    int Window::create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFlags) {

        Uint32 flags = SDL_WINDOW_OPENGL;
        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;

        if (currentFlags & INVISIBLE) {
            flags |= SDL_WINDOW_HIDDEN;
        }
        if (currentFlags & FULLSCREEN) {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        if (currentFlags & BORDERLESS) {
            flags |= SDL_WINDOW_BORDERLESS;
        }

        //Open an SDL window
        m_sdlWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, flags);
        if (m_sdlWindow == nullptr) {
            fatalError("SDL Window could not be created!");
        }

        //Set up our OpenGL context
        SDL_GLContext glContext = SDL_GL_CreateContext(m_sdlWindow);
        if (glContext == nullptr) {
            fatalError("SDL_GL context could not be created!");
        }

        //Set up glew (optional but recommended)
        GLenum error = glewInit();
        if (error != GLEW_OK) {
            fatalError("Could not initialize glew!");
        }

        //Check the OpenGL version
        std::printf("***   OpenGL Version: %s  ***", glGetString(GL_VERSION));


        //Set the background color
        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

        //Set Vsync to on. Unneeded code tho since it was on already by default
        SDL_GL_SetSwapInterval(1);

        //Enable alpha blend
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return 0;
    }

    void Window::swapBuffer() {
        SDL_GL_SwapWindow(m_sdlWindow);
    }

}