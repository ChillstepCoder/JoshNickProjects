#pragma once
#include <vector>
#include "Item.h"

class Inventory {
private:
  std::vector<Item*> items;

public:
  ~Inventory();
  void PrintInventory() const;
  int AddItem(const std::string& name, int value, float weight, ItemType type);
  void RemoveItem(int index);
  const std::vector<Item*>& GetItems() const;
  bool HasWeapon() const;
  void UseItem(int index);
};
