#include "Potion.h"
#include <iostream>

Potion::Potion(const std::string& name, int value, float weight, int uses)
  : Item(name, value, weight = weight*uses ), uses(uses) {}

bool Potion::use() {
  if (uses > 0) {
    float tmpWeight = weight / (float)uses;
    std::cout << "You drink the " << getName() << ".\n\n";
    uses--;
    weight = uses * tmpWeight;
    if (uses == 0) {
      std::cout << "You ran out of " << getName() << ".\n\n";
      return true;
    }
  }
  
  return false;
}

ItemType Potion::getType() const {
  return ItemType::Potion;
}

void Potion::printQuantity() const {
  Item::printQuantity();
  std::cout << " Quantity: " << uses;
}
