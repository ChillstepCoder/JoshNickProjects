#include "Item.h"

Item::Item(string name, int count, int value, int attack, int defense)
{
	_name = name;
	_count = count;
	_value = value;
	_attack = attack;
	_defense = defense;
}

void Item::addOne()
{
	_count++;
}

void Item::removeOne()
{
	if (_count > 0)
	{
		_count--;
	}
}