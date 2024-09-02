//Enemy.h

#pragma once
#include <string>

using namespace std;
class Enemy
{
public:
  Enemy(string name, char tile, int level, int attack, int defense, int health, int xp, int range, int gold, bool mobile);

  int attack();
  int takeDamage(int attack);
  int defenseRoll() const;

  //setters
  void setPosition(int x, int y);

  //getters
  void getPosition(int& x, int& y) const { x = _x; y = _y; }
  int getDefense() const { return _defense; }
  string getName() const { return _name; }
  char getTile() { return _tile; }
  int getXP() const { return _experienceValue; }
  int getGold() const { return _goldValue; }

  //AI
  char getMove(int playerX, int playerY);
  bool isMobile() const { return _mobile; }


private:
  string _name;
  char _tile;

  int _level;
  int _attack;
  int _defense;
  int _health;
  int _experienceValue;
  int _goldValue;
  int _attackRange;
  bool _mobile;

  int _x;
  int _y;
};

