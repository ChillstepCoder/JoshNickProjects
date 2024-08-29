#pragma once
#include <vector>
#include <string>
#include "Soldier.h"

const int NUM_ARMIES = 2;

class Level {
public:
    Level();
    ~Level();

    void load(std::string fileName);
    void printvoid();
    void print();

    void update();

    //Getters
    char getTile(int x, int y);
    Soldier* getSoldier(int x, int y);
    //Setters
    void setTile(int x, int y, char tile, Soldier* soldier);

private:
    void processSoldierMove(char direction, Soldier* soldier);
    void battle(Soldier* soldier, int targetX, int targetY);
    void moveSoldier(Soldier* soldier, int targetX, int targetY);

    std::vector <std::string> _levelData;
    std::vector <std::vector <Soldier* > > _soldierGrid;
    std::vector <Soldier*> _armies[NUM_ARMIES];

};