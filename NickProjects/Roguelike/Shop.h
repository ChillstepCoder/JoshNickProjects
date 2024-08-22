#pragma once
#include <iostream>
#include <string>
#include <list>
#include "Item.h"

class Player;


using namespace std;

class Shop
{
public:
    Shop(string name);
    Shop() = default;

    void enterShop(Player& player);


    void printShop();
    bool canAffordItem(string name, int money);
    bool purchaseItem(string name, Item& newItem);
    void addItem(Item newItem);

    //Getters
    string getName() { return _name; }

private:
    string _name;
    list <Item> _items;
    

};
