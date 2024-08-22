#pragma once
#include <vector>
#include <string>
#include "Player.h"
#include "Enemy.h"
#include "Shop.h"

class Shop;

class Level
{
public:
    Level();
    bool isShopping = true;
    void load(std::string fileName, Player& player, const vector<Shop>& shops);
    void printvoid();
    void print();

    void movePlayer(char input, Player& player);
    void updateEnemies(Player& player);

    //Getters
    char getTile(int x, int y);
    //Setters
    void setTile(int x, int y, char tile);

private:
    void processPlayerMove(Player& player, int targetX, int targetY);
    void processEnemyMove(Player& player, int enemyIndex, int targetX, int targetY);
    void battleMonster(Player& player, int targetX, int targetY);

    std::vector <std::string> _levelData;
    std::vector <Enemy> _enemies;
    std::vector <std::string> _levels;

    int levelnumber = 1;

    vector<Shop> _shops;
};