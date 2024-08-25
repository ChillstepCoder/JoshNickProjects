#pragma once
#include "IItem.h"

class Inventory {
public:

    Inventory();

    void PrintInventory(); //prints each items name and stats
    int AddItem(); //adds an item and returns the index it was added at
    //void RemoveItem(); //removes an item at a specific index
    //const int GetItems(); //returns a const reference to the internal items vector
    //bool HasWeapon(); //returns true if there is at least one weapon. Iterate w/ dynamic_cast to see if weapon exists.

private:
    std::vector <IItem*> items;
    int _numItems;
};
