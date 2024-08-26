#include "Inventory.h"
#include <iostream>
#include <string>
#include <vector>

Inventory::Inventory() { //constructor
	_numItems = 0;
}

Inventory::~Inventory() { //destructor
	for (auto item : _items) {
		delete item;
	}
	_items.clear();
}



void Inventory::PrintInventory() {
	std::cout << "-----------INVENTORY------------\n";
	for (const auto& item : _items) { //iterates through the vector of items to print each one.
		item->print();
	}
	std::cout << "--------------------------------\n";
}

int Inventory::AddItem(std::string Name, int Value, float Weight, int Charges, std::string ItemType) {
	IItem* newItem = nullptr; //creates new item

	if (ItemType == "Sword") { //sets item to its correct type
		newItem = new Sword(Name, Value, Weight);
	}
	else if (ItemType == "Armor") {
		newItem = new Armor(Name, Value, Weight);
	}
	else if (ItemType == "Potion") {
		newItem = new Potion(Name, Value, Weight, Charges);
	}
	else if (ItemType == "Scroll") {
		newItem = new Scroll(Name, Value, Weight, Charges);
	}
	else {
		std::cout << "Invalid item type!\n";
		return -1;
	}

	_items.push_back(newItem); //adds the item to the vector

	_numItems++; //increment the number of items

	return _items.size() - 1; //returns the last index where the item was added
}

void Inventory::RemoveItem(int index) {

	if (index < 0 || index >= _items.size()) { //checks if index is valid
		std::cout << "Invalid index. Cannot remove item.\n";
		return;
	}

	std::string removedItemName = _items[index]->getName(); //store name of item before deleting it


	std::string itemType;
	if (dynamic_cast<Sword*>(_items[index])) { //uses dynamic cast to store the item type
		itemType = "Sword";
	}
	else if (dynamic_cast<Armor*>(_items[index])) {
		itemType = "Armor";
	}
	else if (dynamic_cast<Potion*>(_items[index])) {
		itemType = "Potion";
	}
	else if (dynamic_cast<Scroll*>(_items[index])) {
		itemType = "Scroll";
	}
	else {
		itemType = "Unknown";
	}


	delete _items[index]; //deletes item

	_items.erase(_items.begin() + index); //remove the pointer

	_numItems--; //decrement the number of items

	if (itemType == "Sword" || itemType == "Armor") {
		std::cout << removedItemName << " " << itemType << " removed successfully.\n";
	}
	else if (itemType == "Potion" || itemType == "Scroll") {
		std::cout << itemType << " of " << removedItemName << " removed successfully.\n";
	}
	else {
		std::cout << itemType << " " << removedItemName << " removed successfully.\n";
	}


}

void Inventory::UseItem(int index) {
	if (index < 0 || index >= _items.size()) { //checks if item index is in range.
		std::cout << "Invalid index. Cannot use item.\n";
		return;
	}

	_items[index]->use(); //runs the specific use() command in IItem.cpp

	Potion* potion = dynamic_cast<Potion*>(_items[index]);//converts potion pointer to items pointer
	Scroll* scroll = dynamic_cast<Scroll*>(_items[index]);//converts scroll pointer to items pointer

	if (potion) {
		if (potion->getCharges() <= 0) {
			delete _items[index]; //deletes item

			_items.erase(_items.begin() + index); //remove the pointer

			_numItems--; //decrement the number of items

			std::cout << "You throw the empty bottle away.\n";
		}
	}
	else if (scroll) {
		if (scroll->getCharges() <= 0) {
			delete _items[index]; //deletes item

			_items.erase(_items.begin() + index); //remove the pointer

			_numItems--; //decrement the number of items

			std::cout << "The scroll burns up in the atmosphere.\n";
		}
	}
}

const int Inventory::GetItems() {
	//Implement
	return _items.size();
}

bool Inventory::HasWeapon() {
	//Implement

	return true;
}
