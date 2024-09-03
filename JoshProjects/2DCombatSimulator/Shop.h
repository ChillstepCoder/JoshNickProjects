//Shop.h

#pragma once
#include <string>

class Shop
{
public:
  static const int numItems = 6;
  static const int itemLimits[numItems];
  static const int itemPrices[numItems];
  void printShop();
  void printInventory(const int playerInventory[numItems]);
  std::string getItemName(int index) const;

private:
  std::string shopItemNames[numItems] = { "Helmet", "Sword", "Leggings", "Chestplate", "Food", "Potion" };
};
