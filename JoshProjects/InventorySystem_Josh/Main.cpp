int main() {
  Inventory inventory;

  int swordIndex = inventory.AddItem("Bronze Sword", 100, 2.0f, ItemType::Weapon);
  int armorIndex = inventory.AddItem("Bronze Armor", 150, 65.0f, ItemType::Armor);
  int potionIndex = inventory.AddItem("Potion of Healing", 50, 0.5f, ItemType::Potion);
  int scrollIndex = inventory.AddItem("Scroll of Protection", 25, 0.1f, ItemType::Scroll);

  inventory.PrintInventory();

  inventory.UseItem(potionIndex);

  inventory.PrintInventory();

  inventory.UseItem(potionIndex);

  inventory.PrintInventory();

  inventory.UseItem(potionIndex);

  inventory.UseItem(scrollIndex);

  inventory.PrintInventory();

  std::cout << "Checking for weapon... " << (inventory.HasWeapon() ? "YES" : "NO") << std::endl;

  return 0;
}
