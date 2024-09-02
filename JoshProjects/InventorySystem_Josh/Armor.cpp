#include "Armor.h"
#include <iostream>

Armor::Armor(const std::string& name, int value, float weight)
  : Item(name, value, weight) {}

bool Armor::use() {
  std::cout << "You equip the " << getName() << ".\n";
  return false;
}

ItemType Armor::getType() const {
  return ItemType::Armor;
}
