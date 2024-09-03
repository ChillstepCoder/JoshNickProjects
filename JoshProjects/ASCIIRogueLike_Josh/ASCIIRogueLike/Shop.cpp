#include "Shop.h"
#include <iostream>

const int Shop::itemLimits[Shop::numItems] = { 1, 1, 1, 1, 10, 5 }; // Helmet, Sword, Leggings, Chestplate, Food, Potion
const int Shop::itemPrices[Shop::numItems] = { 50, 100, 50, 75, 10, 30 };

void Shop::printInventory(const int playerInventory[numItems]) {
  std::cout << "\nPlayer Inventory:\n";
  bool hasItems = false;
  for (int i = 0; i < numItems; i++) {
    if (playerInventory[i] > 0) {
      std::cout << shopItemNames[i] << ": " << playerInventory[i] << "\n";
      hasItems = true;
    }
  }
  if (!hasItems) {
    std::cout << "Empty\n";
  }
}

void Shop::printShop() {
  std::cout << "Shop Items:\n";
  for (int i = 0; i < numItems; i++) {
    std::cout << i + 1 << ". " << shopItemNames[i] << " - " << itemPrices[i] << " gold\n";
  }
}

std::string Shop::getItemName(int index) const {
  if (index >= 0 && index < numItems) {
    return shopItemNames[index];
  }
  return "Unknown Item";
}
