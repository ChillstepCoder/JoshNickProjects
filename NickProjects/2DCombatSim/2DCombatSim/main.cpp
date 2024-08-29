#include <iostream>
#include <cstdlib>
#include <string>
#include <conio.h>

#include "GameSystem.h"

int main() {
    GameSystem gameSystem("Level1.txt");

    gameSystem.playGame();

    return 0;
}