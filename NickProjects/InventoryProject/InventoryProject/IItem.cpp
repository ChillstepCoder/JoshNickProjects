#include "IItem.h"


IItem::IItem(std::string Name, int Value, float Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;

}


virtual void IItem::print() {
	std::cout << "Name: " << Name << " Gold: " << Value << " lbs: " << Weight << std::endl;
}


//Weapon//////////////////////////////

Weapon::Weapon(std::string Name, int Value, float Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
}

void Weapon::print() {
	std::cout << Name << " Sword\n" << "Gold: " << Value << "   lbs: " << Weight << std::endl;
}

void Weapon::use() {
	std::cout << "Would you like to equip " << Name << "?\n";
	//implement equip weapon
}

//Armor///////////////////////////////

Armor::Armor(std::string Name, int Value, float Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
}

void Armor::print() {
	std::cout << Name << " Armor\n" << "Gold: " << Value << "    lbs: " << Weight << std::endl;
}

void Armor::use() {
	std::cout << "Would you like to equip " << Name << "?\n";
	//implement equip armor
}

//Potion//////////////////////////////

Potion::Potion(std::string Name, int Value, float Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
}

void Potion::print() {
	std::cout << "Potion of " << Name << "\nGold: " << Value << "    lbs: " << Weight << std::endl;
}

void Potion::use() {
	std::cout << "Glug glug glug\n";
	//implement count decrease
}

//Scroll//////////////////////////////

Scroll::Scroll(std::string Name, int Value, float Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
}

void Scroll::print() {
	std::cout << "Scroll of " << Name << "\nGold: " << Value << "    lbs: " << Weight << std::endl;
}

void Scroll::use() {
	std::cout << "You recite the scripture and the scroll burns up in the atmosphere.\n";
	//implement buff/spell
}


