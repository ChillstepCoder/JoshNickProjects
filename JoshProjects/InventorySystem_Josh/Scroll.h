#pragma once

#include "Item.h"

class Scroll : public Item {
public:
  Scroll(const std::string& name, int value, float weight);
  bool use();
  ItemType getType() const;
};
