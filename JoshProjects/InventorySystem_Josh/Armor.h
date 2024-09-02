#pragma once

#include "Item.h"

class Armor : public Item {
public:
  Armor(const std::string& name, int value, float weight);
  bool use();
  ItemType getType() const;
};
