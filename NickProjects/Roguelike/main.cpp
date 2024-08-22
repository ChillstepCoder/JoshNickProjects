#include <stdio.h>
#include <iostream>
#include <string>
#include "GameSystem.h"

int main()
{

    GameSystem gameSystem("Level1.txt");

    gameSystem.playGame();

    return 0;
}