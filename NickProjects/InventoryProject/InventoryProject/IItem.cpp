#include "IItem.h"


IItem::IItem(std::string Name, int Value, float Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;

}


void IItem::print() {
	std::cout << "Name: " << Name << " Gold: " << Value << " lbs: " << Weight << std::endl;
}


//Weapon//////////////////////////////

Weapon::Weapon(std::string Name, int Value, float Weight) : IItem(Name, Value, Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;

}

void Weapon::print() {
	//make an if statement to determine which type of sword to print. currently everything will print as a sword even if u want one to be a longsword or shortsword
	std::cout << Name << " " << Sword << std::endl << "Gold: " << Value << "   lbs: " << Weight << std::endl;
}

void Weapon::use() {
	//make an if statement to determine which type of sword to print. currently everything will print as a sword even if u want one to be a longsword or shortsword
	std::cout << "You have equipped " << Name << " " << Sword << ".\n";

}

//Armor///////////////////////////////

Armor::Armor(std::string Name, int Value, float Weight) : IItem(Name, Value, Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
}

void Armor::print() {
	std::cout << Name << " Armor\n" << "Gold: " << Value << "   lbs: " << Weight << std::endl;
}

void Armor::use() {
	std::cout << "You have equipped " << Name << " Armor.\n";
	//implement equip armor
}

//Potion//////////////////////////////

Potion::Potion(std::string Name, int Value, float Weight, int Charges) : IItem(Name, Value, Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
	_Charges = Charges;
}

void Potion::print() {
	std::cout << "Potion of " << Name << "\nGold: " << Value << "    lbs: " << Weight << std::endl;
}

void Potion::use() {
	if (_Charges > 0) {
		std::cout << "Glug glug glug\n";
		std::cout << "Your hp has been restored!\n";
		_Charges--;
		std::cout << "Charges remaining: " << _Charges << std::endl;
	}
	else {
		std::cout << "This potion is empty.\n";
	}

}

//Scroll//////////////////////////////

Scroll::Scroll(std::string Name, int Value, float Weight, int Charges) : IItem(Name, Value, Weight) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
	_Charges = Charges;
}

void Scroll::print() {
	std::cout << "Scroll of " << Name << "\nGold: " << Value << "    lbs: " << Weight << std::endl;
}

void Scroll::use() {
	if (_Charges > 0) {
		std::cout << "You recite the scripture on the scroll.\n";
		std::cout << "The blessing of the goddess of protection envelops you!\n";
		_Charges--;
	}
	else {
		std::cout << "This scroll has already been used.\n";
	}

}

