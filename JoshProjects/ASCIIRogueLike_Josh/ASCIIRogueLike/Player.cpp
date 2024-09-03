//Player.cpp

#include "Player.h"
#include <random>
#include <ctime>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

Player::Player() : _level(1), _health(10), _maxHealth(10), _attack(1), _defense(1),
_experience(0), _xpto(10), _x(0), _y(0), _gold(0), _keys(0), _luck(0) {
}

void Player::init(int level, int maxHealth, int attack, int defense, int experience, int xpTo, int luck) {
  _level = level;
  _maxHealth = maxHealth;
  _health = maxHealth;
  _attack = attack;
  _defense = defense;
  _experience = experience;
  _xpto = xpTo;
  _luck = luck;
  _gold = 0;
  _keys = 0;
  updateBonuses();
}


int Player::attack() {
  static std::default_random_engine randomEngine(time(NULL));
  int totalAttack = _attack + _attackBonus;
  std::uniform_int_distribution<int> attackRoll(0, totalAttack + _luck);
  return std::min(attackRoll(randomEngine), totalAttack);
}

//Setters
void Player::setPosition(int x, int y) {
  _x = x;
  _y = y;
}

void Player::addExperience(int experience) {
  _experience += experience;
  while (_experience >= _xpto) {
    _experience -= _xpto;
    _xpto = static_cast<int>((_xpto + 1) * 1.2);
    levelUp();
  }
}

//Getters
void Player::getPosition(int& x, int& y) const {
  x = _x;
  y = _y;
}

int Player::takeDamage(int attack) {
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

int Player::defenseRoll() const {
  static std::default_random_engine randomEngine(time(NULL));
  int totalDefense = _defense + _defenseBonus;
  std::uniform_int_distribution<int> defRoll(1, totalDefense + _luck);
  return std::min(defRoll(randomEngine), totalDefense);
}

bool Player::spendGold(int amount) {
  if (_gold >= amount) {
    _gold -= amount;
    return true;
  }
  return false;
}

void Player::heal(int amount) {
  _health = std::min(_health + amount, _maxHealth);
}

bool Player::useKey() {
  if (_keys > 0) {
    _keys--;
    return true;
  }
  return false;
}


void Player::levelUp() {
  _level++;
  std::cout << "Congratulations! You've reached level " << _level << "!" << std::endl;
  std::cout << "Choose an improvement:" << std::endl;
  std::cout << "1. Increase Health by 5" << std::endl;
  std::cout << "2. Increase Attack by 2" << std::endl;
  std::cout << "3. Increase Defense by 2" << std::endl;
  std::cout << "4. Increase Attack and Defense by 1 each" << std::endl;
  std::cout << "5. Increase Luck by 1" << std::endl;

  std::string input;
  int choice;
  while (true) {
    std::cout << "Enter your choice (1-5): ";
    std::getline(std::cin, input);
    std::stringstream ss(input);
    if (ss >> choice && choice >= 1 && choice <= 5 && ss.eof()) {
      break;
    }
    std::cout << "Invalid input. Please enter a number between 1 and 5." << std::endl;
  }

  switch (choice) {
  case 1: increaseHealth(); break;
  case 2: increaseAttack(); break;
  case 3: increaseDefense(); break;
  case 4: increaseAttackAndDefense(); break;
  case 5: increaseLuck(); break;
  }
  std::cout << "Your stats have been improved!" << std::endl;
}

void Player::increaseLuck() {
  _luck += 1;
}

void Player::increaseHealth() {
  _health += 5;
  _maxHealth += 5;
}

void Player::increaseAttack() {
  _attack += 2;
}

void Player::increaseDefense() {
  _defense += 2;
}

void Player::increaseAttackAndDefense() {
  _attack += 1;
  _defense += 1;
}

void Player::useFood() {
  if (_playerInventory[4] > 0) {
    _playerInventory[4]--;
    heal(5);
    std::cout << "You ate some food and recovered 5 HP." << std::endl;
  }
  else {
    std::cout << "You don't have any food." << std::endl;
  }
}

void Player::usePotion() {
  if (_playerInventory[5] > 0) {
    _playerInventory[5]--;
    heal(_maxHealth / 2);
    std::cout << "You used a potion and recovered " << (_maxHealth / 2) << " HP." << std::endl;
  }
  else {
    std::cout << "You don't have any potions." << std::endl;
  }
}

bool Player::canAddItem(int itemIndex) const {
  return _playerInventory[itemIndex] < Shop::itemLimits[itemIndex];
}

int Player::getItemCount(int itemIndex) const {
  return _playerInventory[itemIndex];
}

void Player::updateBonuses() {
  _attackBonus = _playerInventory[1] * 5;  // Sword gives +5 attack
  _defenseBonus = (_playerInventory[0] + _playerInventory[2] + _playerInventory[3]) * 3;  // Each armor piece gives +3 defense
}

void Player::addToInventory(int itemIndex) {
  if (_playerInventory[itemIndex] < Shop::itemLimits[itemIndex]) {
    _playerInventory[itemIndex]++;
    updateBonuses();
  }
}
