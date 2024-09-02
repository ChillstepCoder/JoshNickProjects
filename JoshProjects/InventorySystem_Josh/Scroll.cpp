#include "Scroll.h"
#include <iostream>

Scroll::Scroll(const std::string& name, int value, float weight)
  : Item(name, value, weight) {}

bool Scroll::use() {
  std::cout << "You read the " << getName() << ". It crumbles to dust.\n";
  return true;
}

ItemType Scroll::getType() const {
  return ItemType::Scroll;
}
