#pragma once
#include <string>
#include <vector>

class Soldier
{
public:
    //Constructor
    Soldier(std::string name, char tile, int level, int attack, int defense, int health, int army);

    //Setters
    void setPosition(int x, int y);

    //Getters
    void getPosition(int& x, int& y);
    std::string getName() { return _name; }
    char getTile() { return _tile; }
    int getArmy() { return _army; }

    int attack();
    int takeDamage(int attack);

    //Gets AI move command
    char getMove(std::vector <Soldier*> armies[], int numArmies);

private:
    Soldier* getClosestEnemy(std::vector <Soldier*> armies[], int numArmies);

    std::string _name;
    char _tile;

    int _level;
    int _attack;
    int _defense;
    int _health;
    int _army;

    //Position
    int _x;
    int _y;
};