#pragma once
#include <string>
#include <vector>
#include "IItem.h"
#include "ItemType.h"

class Inventory {
public:

    Inventory();
    ~Inventory();

    void PrintInventory(); //prints each items name and stats
    int AddItem(std::string Name, int Value, float Weight, int Charges, ItemType itemType); //adds an item and returns the index it was added at
    void RemoveItem(int index); //removes an item at a specific index
    void UseItem(int index); //Uses the item at a specific index
    const int GetItems(); //returns a const reference to the internal items vector
    bool HasWeapon(); //returns true if there is at least one weapon. Iterate w/ dynamic_cast to see if weapon exists.

private:
    std::vector <IItem*> _items;
    int _numItems;
};
