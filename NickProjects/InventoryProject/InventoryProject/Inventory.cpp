#include "Inventory.h"
#include <iostream>
#include <string>
#include <vector>

Inventory::Inventory() {
    _numItems = numItems;
}

void Inventory::PrintInventory() {
    int counter = 0;

    std::cout << "-----------INVENTORY------------\n";
    while (counter < _numItems) {
        std::cout << items.name << ", " << items.value << "g, " << items.weight << "lbs\n";
        counter++;
    }
}

int Inventory::AddItem(std::string Name, int Value, float Weight) {
    int index = 0;


    _numItems++;
    return index;
}

void Inventory::RemoveItem() {

}

const int Inventory::GetItems() {

}

bool HasWeapon() {


    return true;
}
