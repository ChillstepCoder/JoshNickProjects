#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <string>

namespace Bengine {

    enum WindowFlags { INVISIBLE = 0x1, FULLSCREEN = 0x2, BORDERLESS = 0x4 };

    class Window
    {
    public:
        Window();
        ~Window();

        int create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFlags);

        void swapBuffer();

        int getScreenWidth() { m_screenWidth; }
        int getScreenHeight() { m_screenHeight; }

    private:
        SDL_Window* m_sdlWindow;
        int m_screenWidth, m_screenHeight;
    };

}