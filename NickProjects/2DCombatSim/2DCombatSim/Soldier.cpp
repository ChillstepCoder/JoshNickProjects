#include "Soldier.h"

#include <random>
#include <ctime>

Soldier::Soldier(std::string name, char tile, int level, int attack, int defense, int health, int army) {
    _name = name;
    _tile = tile;
    _level = level;
    _attack = attack;
    _defense = defense;
    _health = health;
    _army = army;
}

//Sets the position of the soldier
void Soldier::setPosition(int x, int y) {
    _x = x;
    _y = y;
}

//Gets the position of the soldier using reference variables
void Soldier::getPosition(int& x, int& y) {
    x = _x;
    y = _y;
}

int Soldier::attack() {
    static std::default_random_engine randomEngine(time(NULL));
    std::uniform_int_distribution<int> attackRoll(0, _attack);

    return attackRoll(randomEngine);
}

int Soldier::takeDamage(int attack) {
    attack -= _defense;
    //check if the attack does damage
    if (attack > 0) {
        _health -= attack;
        //check if he died
        if (_health <= 0) {
            return 1;
        }

    }
    return 0;
}

char Soldier::getMove(std::vector <Soldier*> armies[], int numArmies) {
    Soldier* closestSoldier = getClosestEnemy(armies, numArmies);

    if (closestSoldier == nullptr) {
        return '.';
    }
    int soldierX, soldierY;

    closestSoldier->getPosition(soldierX, soldierY);

    int dx = _x - soldierX;
    int dy = _y - soldierY;
    int adx = abs(dx);
    int ady = abs(dy);


    //Moving along X axis
    if (adx > ady) {
        if (dx > 0) {
            return 'a'; //Move left
        }
        else {
            return 'd'; //Move right
        }
    }
    //Moving along Y axis
    else {
        if (dy > 0) {
            return 'w'; //Move up
        }
        else {
            return 's'; //Move down
        }
    }
}


Soldier* Soldier::getClosestEnemy(std::vector <Soldier*> armies[], int numArmies) {
    Soldier* closestSoldier = nullptr;
    int closestDistance = INT_MAX;
    int enemyX, enemyY;
    int distance;
    for (int i = 0; i < numArmies; i++) {
        if (i != _army) {
            for (int j = 0; j < armies[i].size(); j++) {
                armies[i][j]->getPosition(enemyX, enemyY);

                distance = abs(enemyX - _x) + abs(enemyY - _y);
                if (distance < closestDistance) {
                    closestSoldier = armies[i][j];
                    closestDistance = distance;
                }
            }
        }
    }

    return closestSoldier;

}