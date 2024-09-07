//JAGEngine.cpp

#include "GL/glew.h"
#include "SDL/SDL.h"
#include "JAGEngine.h"

namespace JAGEngine {
  int init() {
    //initialize SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    return 0;
  }
}
