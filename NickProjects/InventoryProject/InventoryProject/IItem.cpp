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

Weapon::Weapon(std::string Name, int Value, float Weight, WeaponType type) : IItem(Name, Value, Weight), _type(type) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;

}

void Weapon::print() {
	//make an if statement to determine which type of sword to print. currently everything will print as a sword even if u want one to be a longsword or shortsword
	std::cout << Name << " " << getWeaponTypeName(_type) << std::endl << "Gold: " << Value << "   lbs: " << Weight << std::endl;
}

void Weapon::use() {
	//make an if statement to determine which type of sword to print. currently everything will print as a sword even if u want one to be a longsword or shortsword
	std::cout << "You have equipped " << Name << " " << getWeaponTypeName(_type) << ".\n";

}

std::string Weapon::getWeaponTypeName(WeaponType type) const {
	switch (type) {
		case WeaponType::Sword: return "Sword";
		case WeaponType::LongSword: return "LongSword";
		case WeaponType::ShortSword: return "ShortSword";
		default: return "Unknown";

	}
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

Potion::Potion(std::string Name, int Value, float Weight, int Charges, PotionSize size) : IItem(Name, Value, Weight), _size(size) {
	IItem::Name = Name; // IItem::    = scope resolution operator
	IItem::Value = Value;
	IItem::Weight = Weight;
	_Charges = Charges;
}

void Potion::print() {
	std::cout << getPotionSizeName(_size) << " potion of " << Name << "\nGold: " << Value << "    lbs: " << Weight << std::endl;
}

void Potion::use() {
	if (_Charges > 0) {
		std::cout << "Glug glug glug\n";
		switch (_size) {
			case PotionSize::Small:
				std::cout << "You feel slightly better\n";
				break;
			case PotionSize::Medium:
				std::cout << "You feel better.\n";
				break;
			case PotionSize::Large:
				std::cout << "You feel much better.\n";
				break;
			case PotionSize::Grand:
				std::cout << "You feel completely restored!\n";
				break;
		}
		_Charges--;
		std::cout << "Charges remaining: " << _Charges << std::endl;
	}
	else {
		std::cout << "This potion is empty.\n";
	}

}

std::string Potion::getPotionSizeName(PotionSize size) const {
	switch (size) {
	case PotionSize::Small: return "Small";
	case PotionSize::Medium: return "Medium";
	case PotionSize::Large: return "Large";
	case PotionSize::Grand: return "Grand";
	default: return "Unknown";

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

