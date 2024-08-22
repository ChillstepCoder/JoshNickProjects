#include <iostream>
#include <string>
#include "GameSystem.h"
#include "Shop.h"
#include <conio.h>


// Check if we are on Windows
#if defined(_WIN32) || defined(_win64)
#include <conio.h>
#define HAS_GETCH 1
#else
#define HAS_GETCH 0
#endif


//Constructor sets up the game
GameSystem::GameSystem(std::string levelFileName)
{
    _player.init(1, 30, 5, 2, 0, 0, 0); //level, health, attack, defense, experience, weaponatk, armordef)
    vector<Shop> shops;
    initShops(shops);
    

    _level.load(levelFileName, _player, shops);

}

void GameSystem::playGame()
{
    bool isDone = false;



    while (isDone != true)
    {

        _level.printvoid();
        _player.printstats();
        _level.print();
        playerMove();
        _level.updateEnemies(_player);
    }

}

void GameSystem::playerMove()
{
    char input;

    printf("Enter a move command (w/s/a/d): ");

#if HAS_GETCH
    char c = _getch(); // Use _getch() on Windows
#else
    char c = std::cin.get(); // Use std::cin.get() on other platforms
#endif

    input = c;
    _level.movePlayer(input, _player);

}



void GameSystem::initShops(vector<Shop>& shops)
{
    shops.push_back(Shop("The Blacksmith"));
    shops.back().addItem(Item("Dagger", 1, 50, 2, 0));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Short Sword", 1, 150, 4, 0));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Long Sword", 1, 400, 10, 0));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Knight Sword", 1, 1000, 25, 0));//(Name, Count, Value, Atk, Def)

    shops.push_back(Shop("The Armory"));
    shops.back().addItem(Item("Leather Armor", 1, 50, 0, 2));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Copper Armor", 1, 150, 0, 4));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Iron Armor", 1, 400, 0, 10));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Knight Armor", 1, 1000, 0, 25));//(Name, Count, Value, Atk, Def)


    shops.push_back(Shop("The Forge of the Gods"));
    shops.back().addItem(Item("Divine Short Sword", 1, 3000, 125, 0));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Divine Long Sword", 1, 15000, 400, 0));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Divine Knight Sword", 1, 40000, 800, 0));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Excalibur", 1, 200000, 2000, 0));//(Name, Count, Value, Atk, Def)

    shops.push_back(Shop("The Divine Reliquary"));
    shops.back().addItem(Item("Cobalt Armor", 1, 3000, 0, 125));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Mythril Armor", 1, 15000, 0, 400));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Adamantite Armor", 1, 40000, 0, 800));//(Name, Count, Value, Atk, Def)
    shops.back().addItem(Item("Divine Knight Armor", 1, 200000, 0, 2000));//(Name, Count, Value, Atk, Def)

}