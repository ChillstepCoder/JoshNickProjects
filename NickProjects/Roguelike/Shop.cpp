#include "Shop.h"
#include "Item.h"
#include <iostream>
#include <string>
#include "Player.h"

Shop::Shop(string name)
{
	_name = name;
}


void Shop::enterShop(Player &player)
{
	bool isDone = false;
	string itemName;
	Item newItem("NO ITEM", 0, 0, 0, 0);
	std::cin.clear();
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	while (isDone == false) {

		std::cout << std::string(100, '\n');
		printShop();
		player.printstats();
		player.printInventory();

		cout << "\nType the name of the item you wish to buy.\n\nType Quit to leave the shop.\n\n";
		
		getline(cin, itemName);

		if (itemName == "Quit" || itemName == "quit")
		{
			isDone = true;
		}
		else
		{
			if (canAffordItem(itemName, player.getGold()))
			{
				if (getName() == "The Blacksmith" || getName() == "The Forge of the Gods")
				{
					//Weapon purchase
					if (purchaseItem(itemName, newItem) == true)
					{
						player.addGold(-newItem.getValue());
						player.addItemWeapon(newItem);
					}
					else
					{
						cout << "That is an invalid item!\n";
						system("PAUSE");
					}
				}
				else
				{
					//Armor purchase
					if (purchaseItem(itemName, newItem) == true)
					{
						player.addGold(-newItem.getValue());
						player.addItemArmor(newItem);

					}
					else
					{
						cout << "That is an invalid item!\n";
						system("PAUSE");
					}
				}
			}
			else
			{
				cout << "You don't have enough money!\n";
				system("PAUSE");
			}
		}

	}
}

void Shop::printShop()
{
	std::cout << "*** Welcome to the item shop! ***\n";
	list<Item>::iterator lit;

	int i = 0;

	for (lit = _items.begin(); lit != _items.end(); lit++)
	{
		cout << "Price: " << (*lit).getValue() << "g - " << (*lit).getName() << endl;
		i++;
	}
	cout << "*********************************\n\n";
}


bool Shop::canAffordItem(string name, int gold)
{
	list<Item>::iterator lit;

	for (lit = _items.begin(); lit != _items.end(); lit++) {
		if ((*lit).getName() == name) {
			if ((*lit).getValue() <= gold) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	return false;
}

bool Shop::purchaseItem(string name, Item &newItem)
{
	list<Item>::iterator lit;
	for (lit = _items.begin(); lit != _items.end(); lit++)
	{
		if ((*lit).getName() == name)
		{
			
			newItem = (*lit);
			newItem.setCount(1);
			(*lit).removeOne();

			if ((*lit).getCount() == 0)
			{
				_items.erase(lit);
			}
			return true;
		}
	}
	return false;
}

void Shop::addItem(Item newItem)
{
	list<Item>::iterator lit;
	for (lit = _items.begin(); lit != _items.end(); lit++)
	{
		if ((*lit).getName() == newItem.getName())
		{
			(*lit).addOne();
			return;
		}
	}

	_items.push_back(newItem);
}
