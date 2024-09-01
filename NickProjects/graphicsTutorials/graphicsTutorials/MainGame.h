#pragma once

#include <SDL/SDL.h>

class MainGame
{
public:
    MainGame();
    ~MainGame();

    void run();

    void initSystems();

private:
    SDL_Window* _window;
    int _screenWidth;
    int _screenHeight;
};

