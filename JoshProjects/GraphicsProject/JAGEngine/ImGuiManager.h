//ImGuiManager.h

#pragma once

#include <SDL/SDL.h>
#include "Window.h"

namespace JAGEngine {

  class ImGuiManager {
  public:
    static void init(Window* window);
    static void shutdown();
    static void newFrame();
    static void render();
    static void processEvent(SDL_Event& event);

  private:
    static bool s_initialized;
  };

}
