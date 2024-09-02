#include "Weapon.h"
#include <iostream>

Weapon::Weapon(const std::string& name, int value, float weight)
  : Item(name, value, weight) {}

bool Weapon::use() {
  std::cout << "You swing the " << getName() << ".\n";
  return false;
}

ItemType Weapon::getType() const {
  return ItemType::Weapon;
}
