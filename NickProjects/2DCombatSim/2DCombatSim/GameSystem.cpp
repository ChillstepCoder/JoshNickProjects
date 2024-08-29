#include "GameSystem.h"
#include <iostream>
#include <conio.h>

//Constructor sets up the game
GameSystem::GameSystem(std::string levelFileName) {

    //Load the level from file
    _level.load(levelFileName);

}

void GameSystem::playGame() {
    bool isDone = false;



    while (isDone != true) {

        _level.printvoid();
        _level.print();

        _level.update();

        _getch();

    }

}