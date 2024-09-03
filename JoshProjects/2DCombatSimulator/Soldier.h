#pragma once
#include <string>
#include <vector>

class Soldier
{
public:
  Soldier(std::string name, char tile, int level, int attack, int defense, int health, int army, int range, int gold, bool mobile);
  int attack();
  int takeDamage(int attack);
  int defenseRoll() const;
  void setPosition(int x, int y);
  void setMoves(bool left, bool right, bool up, bool down);

  void getPosition(int& x, int& y) const { x = _x; y = _y; }
  int getDefense() const { return _defense; }
  std::string getName() const { return _name; }
  char getTile() const { return _tile; }
  int getTeam() const { return _army; }
  int getGold() const { return _goldValue; }
  int getHealth() const { return _health; }
  char getMove(std::vector <Soldier*> armies[], int numArmies);
  bool isMobile() const { return _mobile; }

private:
  Soldier* getClosestEnemy(std::vector <Soldier*> armies[], int numArmies);
  std::string _name;
  char _tile;
  int _level;
  int _attack;
  int _defense;
  int _health;
  int _army;
  int _goldValue;
  int _attackRange;
  bool _mobile;
  int _x;
  int _y;
  bool _leftBlocked;
  bool _rightBlocked;
  bool _upBlocked;
  bool _downBlocked;
};
