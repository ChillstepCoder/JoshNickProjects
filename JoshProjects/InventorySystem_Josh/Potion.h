#pragma once

#include "Item.h"

class Potion : public Item {
private:
  int uses;

public:
  Potion(const std::string& name, int value, float weight, int uses);
  bool use();
  ItemType getType() const;
  void printQuantity() const;
};
