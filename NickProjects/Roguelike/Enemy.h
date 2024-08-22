#pragma once
#include <string>
#include "Player.h"

class Enemy
{
public:
    Enemy(std::string name, char tile, int level, int attack, int defense, int health, int xp);

    //Setters
    void setPosition(int x, int y);

    //Getters
    void getPosition(int& x, int& y);
    std::string getName() { return _name; }
    char getTile() { return _tile; }
    int getLevel() { return _level; }
    int getAttack() { return _attack; }
    int getDefense() { return _defense; }
    int getHealth() { return _health; }
    int getExperience() { return _experienceValue; }
    int getGold() { return _gold; }

    int attack();
    int takeDamage(int attack);

    //Gets AI move command
    char getMove(int playerX, int playerY);

private:
    std::string _name;
    char _tile;

    int _level;
    int _attack;
    int _defense;
    int _health;
    int _experienceValue;
    int _gold;

    //Position
    int _x;
    int _y;
};