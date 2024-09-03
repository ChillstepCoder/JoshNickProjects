#pragma once

#include "Item.h"

class Weapon : public Item {
public:
  Weapon(const std::string& name, int value, float weight);
  bool use();
  ItemType getType() const;
};
