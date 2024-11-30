#include "Window.h"
#include  "BengineErrors.h"
#include "DebugOpenGL.h"

namespace Bengine {

    Window::Window() {

    }
    Window::~Window() {
        if (m_sdlWindow != nullptr) {
            SDL_DestroyWindow(m_sdlWindow);
        }
    }

    int Window::create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFlags) {
        Uint32 flags = SDL_WINDOW_OPENGL | currentFlags;

        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;

        if (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) {
            SDL_DisplayMode DM;
            SDL_GetCurrentDisplayMode(0, &DM);
            auto Width = DM.w;
            auto Height = DM.h;
            m_screenWidth = Width;
            m_screenHeight = Height - 128;
            // Strip out fullscreen flag
            flags &= ~(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP);
        }

        //Open an SDL window

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        m_sdlWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, flags);
        if (m_sdlWindow == nullptr) {
            fatalError("SDL Window could not be created!");
        }

        SDL_GetWindowSize(m_sdlWindow, &m_screenWidth, &m_screenHeight);

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

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(DebugOpenGL::glDebugOutput, nullptr);

        // Filter out notification messages
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

        //Check the OpenGL version
        std::printf("***   OpenGL Version: %s  ***", glGetString(GL_VERSION));


        //Set the background color
        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

        //Set Vsync to on. Unneeded code tho since it was on already by default
        SDL_GL_SetSwapInterval(1);

        //Enable alpha blend
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Make sure window is shown and raised
        SDL_ShowWindow(m_sdlWindow);
        SDL_RaiseWindow(m_sdlWindow);

        // Set window to not minimized
        SDL_SetWindowMinimumSize(m_sdlWindow, 640, 480);  // Set minimum size

        return 0;
    }

    void Window::swapBuffer() {
        SDL_GL_SwapWindow(m_sdlWindow);
    }

}