#pragma once
#include "Level.h"
#include <string>

class GameSystem
{
public:
    GameSystem(std::string levelFileName);

    void playGame();

private:
    Level _level;
};