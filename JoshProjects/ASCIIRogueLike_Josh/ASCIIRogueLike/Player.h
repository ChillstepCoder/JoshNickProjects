//Player.h
#include "Shop.h"
#pragma once
using namespace std;

class Player
{
public:
  Player();
  void init(int level, int health, int attack, int defence, int experience, int xpto, int luck);

  int attack();
  int takeDamage(int attack);
  int defenseRoll() const;
  void heal(int amount);
  bool useKey();
  void levelUp();
  void useFood();
  void usePotion();
  bool canAddItem(int itemIndex) const;

  //Setters
  void setPosition(int x, int y);
  void addExperience(int experience);
  void addGold(int amount) { _gold += amount; }
  bool spendGold(int amount);
  void addToInventory(int itemIndex);
  void addKey() { _keys++; }
  void setHealth(int health) { _health = std::min(health, _maxHealth); }

  //Getters
  int getDefense() const { return _defense; }
  int getHealth() const { return _health; }
  int getMaxHealth() const { return _maxHealth; }
  int getLevel() const { return _level; }
  int getAttack() const { return _attack; }
  int getExperience() const { return _experience; }
  int getXpToNextLevel() const { return _xpto; }
  void getPosition(int& x, int& y) const;
  int getGold() const { return _gold; }
  const int* getInventory() const { return _playerInventory; }
  int getKeys() const { return _keys; }
  int getLuck() const { return _luck; }
  int getItemCount(int itemIndex) const;
  int getAttackBonus() const { return _attackBonus; }
  int getDefenseBonus() const { return _defenseBonus; }
  int getLuckBonus() const { return _luckBonus; }
  

private:
  void updateBonuses();
  void increaseHealth();
  void increaseAttack();
  void increaseDefense();
  void increaseAttackAndDefense();
  void increaseLuck();

  int _level;
  int _health;
  int _maxHealth;
  int _attack;
  int _defense;
  int _luck;
  int _experience;
  int _xpto;
  int _playerInventory[Shop::numItems] = { 0 };
  int _gold;
  int _keys;

  int _attackBonus = 0;
  int _defenseBonus = 0;
  int _luckBonus = 0;

  int _x;
  int _y;

  

};
