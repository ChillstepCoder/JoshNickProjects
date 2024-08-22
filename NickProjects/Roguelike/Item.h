#pragma once
#include <string>

using namespace std;

class Item
{
public:
	Item(string name, int value, int count, int attack, int defense);

	void addOne();
	void removeOne();

	//Getters
	string getName() { return _name; }
	int getValue() { return _value; }
	int getCount() { return _count; }
	int getAttack() { return _attack; }
	int getDefense() { return _defense; }

	//Setters
	void setCount(int count) { _count = count; }


private:
	string _name;
	int _value;
	int _count;
	int _attack;
	int _defense;
};

