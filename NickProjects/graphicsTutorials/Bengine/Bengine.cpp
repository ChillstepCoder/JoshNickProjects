#include <SDL/SDL.h>
#include <GL/glew.h>
#include "Bengine.h"

namespace Bengine {

    int init() {
        //Initialize SDL
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_HAPTIC);

        //Tell SDL that we want a double buffered window so we dont get any flickering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        return 0;
    }

}