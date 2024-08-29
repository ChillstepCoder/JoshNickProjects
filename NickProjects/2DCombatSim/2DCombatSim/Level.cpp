#include "Level.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <random>

Level::Level() {

}

Level::~Level() {
    for (int i = 0; i < NUM_ARMIES; i++) {
        for (int j = 0; j < _armies[i].size(); j++) {
            delete _armies[i][j];
        }
    }
}

void Level::load(std::string fileName) {
    //Loads the level

    std::ifstream file;

    file.open(fileName);
    if (file.fail()) {
        perror(fileName.c_str());
        system("PAUSE");
        exit(1);
    }

    std::string line;

    while (getline(file, line)) {
        _levelData.push_back(line);
        _soldierGrid.push_back(std::vector <Soldier* >());
        _soldierGrid.back().resize(line.size(), nullptr);
    }

    file.close();

    //Process the level
    char tile;

    for (int i = 0; i < _levelData.size(); i++) {
        for (int j = 0; j < _levelData[i].size(); j++) {
            tile = _levelData[i][j];

            switch (tile) {
            case '1': //Team 1
                _armies[0].push_back(new Soldier("1", tile, 1, 10, 5, 10, 0)); //level, attack, defense, health, xp)
                _armies[0].back()->setPosition(j, i);
                _soldierGrid[i][j] = _armies[0].back();
                break;
            case '2': //Soldier 2
                _armies[1].push_back(new Soldier("2", tile, 1, 10, 5, 10, 1)); //level, attack, defense, health, xp)
                _armies[1].back()->setPosition(j, i);
                _soldierGrid[i][j] = _armies[1].back();
                break;
            case '#':
            case '.':
                break;
            default:
                printf("WARNING: Unknown tile %c at %d,%d", tile, j, i);
                system("PAUSE");
                break;

            }
        }
    }

    std::random_device::result_type seed = std::random_device()();
    std::mt19937 randomEngine(seed);
    Soldier* tmp;


    //Shuffle armies
    for (int i = 0; i < NUM_ARMIES; i++) {
        //Iterate backwards through vector
        for (int j = _armies[i].size() - 1; j > 1; j--) {
            std::uniform_int_distribution<int> randomSwap(0, j - 1);
            int swap = randomSwap(randomEngine);
            tmp = _armies[i][j];
            _armies[i][j] = _armies[i][swap];
            _armies[i][swap] = tmp;
        }
    }

}
void Level::printvoid() {
    std::cout << std::string(100, '\n');
}

void Level::print() {
    printf("\n");

    for (int i = 0; i < _levelData.size(); i++) {
        printf("%s\n", _levelData[i].c_str());
    }
    printf("\n");
}



void Level::update() {
    char move;
    int i = 0;
    bool isDone = false;
    //loops until done
    while (isDone == false) {
        isDone = true;
        //Loops through all armies
        for (int j = 0; j < NUM_ARMIES; j++) {
            if (i < _armies[j].size()) {
                move = _armies[j][i]->getMove(_armies, NUM_ARMIES);
                processSoldierMove(move, _armies[j][i]);
                isDone = false;
            }
        }
        i++;
    }
}

char Level::getTile(int x, int y) {
    return _levelData[y][x];
}

Soldier* Level::getSoldier(int x, int y) {
    return _soldierGrid[y][x];
}

void Level::setTile(int x, int y, char tile, Soldier* soldier) {
    _levelData[y][x] = tile;
    _soldierGrid[y][x] = soldier;
}


void Level::processSoldierMove(char direction, Soldier* soldier) {
    int x, y;
    int targetX, targetY;

    soldier->getPosition(x, y);

    switch (direction) {
    case 'w': //Up
        targetX = x;
        targetY = y - 1;
        break;
    case 'a': //Left
        targetX = x - 1;
        targetY = y;
        break;
    case 's': //Down
        targetX = x;
        targetY = y + 1;
        break;
    case 'd': //Right
        targetX = x + 1;
        targetY = y;
        break;
    case '.': //No enemies left
        return;
    }

    char targetTile = getTile(targetX, targetY);

    switch (targetTile) {
    case '#':
        break;
    case '.':
        moveSoldier(soldier, targetX, targetY);
        break;
    default:
        battle(soldier, targetX, targetY);
    }

}

void Level::battle(Soldier* soldier, int targetX, int targetY) {
    int x, y;
    int enemyArmy;
    soldier->getPosition(x, y);

    Soldier* targetSoldier = getSoldier(targetX, targetY);
    if (targetSoldier == nullptr) { //this shouldnt ever happen anyways
        return;
    }
    enemyArmy = targetSoldier->getArmy();
    if (enemyArmy == soldier->getArmy()) { // If the target is on the same team

        return;
    }

    //Enemy team - ATTACK
    int result = targetSoldier->takeDamage(soldier->attack());
    if (result == 1) {
        for (int i = 0; i < _armies[enemyArmy].size(); i++) {
            if (_armies[enemyArmy][i] == targetSoldier) {
                _armies[enemyArmy][i] = _armies[enemyArmy].back();
                _armies[enemyArmy].pop_back();

                delete targetSoldier;

                setTile(targetX, targetY, '.', nullptr);
                break;
            }
        }
    }
}


void Level::moveSoldier(Soldier* soldier, int targetX, int targetY) {
    int x, y;
    soldier->getPosition(x, y);

    setTile(x, y, '.', nullptr);
    setTile(targetX, targetY, soldier->getTile(), soldier);

    soldier->setPosition(targetX, targetY);
}