//Enemy.cpp

#include "Enemy.h"
#include <random>
#include <ctime>

Enemy::Enemy(string name, char tile, int level, int attack, int defense, int health, int xp, int range, int gold, bool mobile)
  : _name(name), _tile(tile), _level(level), _attack(attack), _defense(defense),
  _health(health), _experienceValue(xp), _attackRange(range), _goldValue(gold), _x(0), _y(0), _mobile(mobile) {
}

int Enemy::attack() {
  static default_random_engine randomEngine(time(NULL));
  uniform_int_distribution<int> attackRoll(0, _attack);
  return attackRoll(randomEngine);
}

int Enemy::takeDamage(int attack) {
  if (attack == 0) return 0;
  int defenseValue = defenseRoll();
  int damage = attack / defenseValue;
  if (damage > 0) {
    _health -= damage;
    if (_health <= 0) {
      return _experienceValue;
    }
  }
  return 0;
}

int Enemy::defenseRoll() const {
  static default_random_engine randomEngine(time(NULL));
  uniform_int_distribution<int> defRoll(1, _defense);
  return defRoll(randomEngine);
}

void Enemy::setPosition(int x, int y) {
  _x = x;
  _y = y;
}

char Enemy::getMove(int playerX, int playerY) {
  if (!_mobile) {
    return '.';
  }
  static default_random_engine randomEngine(time(NULL));
  uniform_int_distribution<int> moveRoll(0, 6);
  int distance;
  int dx = _x - playerX;
  int dy = _y - playerY;
  int adx = abs(dx);
  int ady = abs(dy);
  distance = adx + ady;
  if (distance <= _attackRange) {
      //x axis
    if (adx > ady) {
      if (dx > 0) {
        return 'a';
      } else {
          return 'd';
      }
    } else {
      //y axis
      if (dy > 0) {
        return 'w';
      } else {
          return 's';
      }
    }
  }

  int randomMove = moveRoll(randomEngine);
  switch (randomMove) {
  case 0:
    return 'a';
  case 1:
    return 'w';
  case 2:
    return 's';
  case 3:
    return 'd';
  default:
    return '.';
  }
}

