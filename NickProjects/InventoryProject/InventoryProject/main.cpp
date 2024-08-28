#include "Inventory.h"
#include "ItemType.h"

int main()
{

    Inventory Inventory;
    Inventory.AddItem("Bronze", 100, 2.0f, 0, ItemType::Weapon, Weapon::WeaponType::Sword);    //(Name, Value(g), Weight(lbs), charges, itemType, WeaponType)
    Inventory.AddItem("Bronze", 150, 65.0f, 0, ItemType::Armor);   //(Name, Value(g), Weight(lbs), charges, itemType)
    Inventory.AddItem("Healing", 50, 0.5f, 3, ItemType::Potion, Weapon::WeaponType::Sword, Potion::PotionSize::Small);   //(Name, Value(g), Weight(lbs), charges, itemType, WeaponType, PotionSize)
    Inventory.AddItem("Protection", 25, 0.1f, 1, ItemType::Scroll);//(Name, Value(g), Weight(lbs), charges, itemType)
    Inventory.PrintInventory();
    Inventory.RemoveItem(1);
    Inventory.PrintInventory();
    Inventory.UseItem(1);
    Inventory.UseItem(1);
    Inventory.UseItem(1);
    Inventory.PrintInventory();
    Inventory.UseItem(0);
    Inventory.UseItem(1);
    Inventory.PrintInventory();
    Inventory.AddItem("Diamond", 15000, 6.5f, 0, ItemType::Weapon, Weapon::WeaponType::LongSword);
    Inventory.UseItem(1);
    Inventory.PrintInventory();
    Inventory.AddItem("Healing", 1200, 0.5f, 3, ItemType::Potion, Weapon::WeaponType::Sword, Potion::PotionSize::Grand);
    Inventory.UseItem(2);
    int tmp;
    std::cin >> tmp;

    return 0;
}