#pragma once
#include <SDL/SDL.h>
#include "Window.h"

namespace Bengine {

    class ImGuiManager {
    public:
        static void init(Window* window);
        static void shutdown();
        static void newFrame();
        static void renderFrame();
        static void processEvent(SDL_Event& event);

    private:
        inline static bool s_initialized = false;
        inline static Window* s_window;
    };

}