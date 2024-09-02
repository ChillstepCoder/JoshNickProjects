#include "Soldier.h"
#include <random>
#include <ctime>

Soldier::Soldier(std::string name, char tile, int level, int attack, int defense, int health, int army, int range, int gold, bool mobile)
  : _name(name), _tile(tile), _level(level), _attack(attack), _defense(defense),
  _health(health), _army(army), _attackRange(range), _goldValue(gold), _x(0), _y(0), _mobile(mobile) {
}

int Soldier::attack() {
  static std::default_random_engine randomEngine(std::time(nullptr));
  std::uniform_int_distribution<int> attackRoll(0, _attack);
  return attackRoll(randomEngine);
}

int Soldier::takeDamage(int attack) {
  if (attack == 0) return 0;
  int defenseValue = defenseRoll();
  int damage = attack / defenseValue;
  if (damage > 0) {
    _health -= damage;
    if (_health <= 0) {
      return 1;
    }
  }
  return 0;
}

int Soldier::defenseRoll() const {
  static std::default_random_engine randomEngine(std::time(nullptr));
  std::uniform_int_distribution<int> defRoll(1, _defense);
  return defRoll(randomEngine);
}

void Soldier::setPosition(int x, int y) {
  _x = x;
  _y = y;
}

char Soldier::getMove(std::vector<Soldier*> armies[], int numArmies) {
  Soldier* closestSoldier = getClosestEnemy(armies, numArmies);
  if (closestSoldier == nullptr) {
    return '.';
  }

  int soldierX, soldierY;
  closestSoldier->getPosition(soldierX, soldierY);

  int dx = soldierX - _x;
  int dy = soldierY - _y;
  int adx = std::abs(dx);
  int ady = std::abs(dy);

  // attempt preferred move
  if (adx > ady) {
    if (dx > 0 && !_rightBlocked) return 'd';
    if (dx < 0 && !_leftBlocked) return 'a';
  }
  else {
    if (dy > 0 && !_downBlocked) return 's';
    if (dy < 0 && !_upBlocked) return 'w';
  }

  // try other directions
  if (adx > ady) {
    if (!_downBlocked) return 's';
    if (!_upBlocked) return 'w';
    if (dx > 0 && !_leftBlocked) return 'a';
    if (dx < 0 && !_rightBlocked) return 'd';
  }
  else {
    if (!_rightBlocked) return 'd';
    if (!_leftBlocked) return 'a';
    if (dy > 0 && !_upBlocked) return 'w';
    if (dy < 0 && !_downBlocked) return 's';
  }

  return '.';
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

void Soldier::setMoves(bool left, bool right, bool up, bool down) {
  _leftBlocked = !left;
  _rightBlocked = !right;
  _upBlocked = !up;
  _downBlocked = !down;
}
