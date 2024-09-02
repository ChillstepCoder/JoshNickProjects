#include "Inventory.h"
#include "Weapon.h"
#include "Armor.h"
#include "Potion.h"
#include "Scroll.h"
#include <iostream>

Inventory::~Inventory() {
  for (size_t i = 0; i < items.size(); ++i) {
    delete items[i];
  }
}

void Inventory::PrintInventory() const {
  std::cout << "======== INVENTORY ========\n";
  for (size_t i = 0; i < items.size(); ++i) {
    std::cout << i << ": ";
    items[i]->printQuantity();
    std::cout << std::endl;
  }
  std::cout << "===========================\n\n";
}

int Inventory::AddItem(const std::string& name, int value, float weight, ItemType type) {
  Item* newItem = nullptr;
  switch (type) {
  case ItemType::Weapon:
    newItem = new Weapon(name, value, weight);
    break;
  case ItemType::Armor:
    newItem = new Armor(name, value, weight);
    break;
  case ItemType::Potion:
    newItem = new Potion(name, value, weight, 3); // Uses
    break;
  case ItemType::Scroll:
    newItem = new Scroll(name, value, weight);
    break;
  }
  if (newItem) {
    items.push_back(newItem);
    return items.size() - 1;
  }
  return -1;
}

void Inventory::RemoveItem(int index) {
  if (index >= 0 && index < static_cast<int>(items.size())) {
    delete items[index];
    items.erase(items.begin() + index);
  }
}

const std::vector<Item*>& Inventory::GetItems() const {
  return items;
}

bool Inventory::HasWeapon() const {
  for (size_t i = 0; i < items.size(); ++i) {
    if (items[i]->getType() == ItemType::Weapon) {
      return true;
    }
  }
  return false;
}

void Inventory::UseItem(int index) {
  if (index >= 0 && index < static_cast<int>(items.size())) {
    bool shouldRemove = items[index]->use();
    if (shouldRemove) {
      RemoveItem(index);
    }
  }
}
