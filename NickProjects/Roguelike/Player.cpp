#include "Player.h"
#include "Level.h"
#include <random>
#include <ctime>
#include <iostream>

//Constructor, just initializes position to zero
Player::Player() {
    _x = 0;
    _y = 0;
}

//Initializes the player with properties
void Player::init(int level, int health, int attack, int defense, int experience, int weaponatk, int armordef) {
    _level = level;
    _maxhealth = health;
    _health = health;
    _attack = (attack + weaponatk);
    _defense = (defense + armordef);
    _experience = experience;
    _weaponatk = weaponatk;
    _armordef = armordef;

    //addItemWeapon(Item("Broken Dagger", 1, 0, 1, 0));//(Name, Count, Value, Atk, Def)
    //addItemArmor(Item("Torn Shirt", 1, 0, 0, 1));//(Name, Count, Value, Atk, Def)
}

int Player::attack()
{
    static std::default_random_engine randomEngine(time(NULL));
    std::uniform_int_distribution<int> attackRoll(0, (_attack + _weaponatk));

    return attackRoll(randomEngine);
}

int Player::takeDamage(int attack)
{
    attack -= (_defense + _armordef);
    //check if the attack does damage
    if (attack > 0)
    {
        _health -= attack;
        //check if he died
        if (_health <= 0)
        {
            return 1;
        }

    }
    return 0;
}

bool Player::canAffordItem(string name, int money)
{
    list<Item>::iterator lit;

    for (lit = _items.begin(); lit != _items.end(); lit++) {
        if ((*lit).getName() == name) {
            if ((*lit).getValue() <= money) {
                return true;
            }
            else {
                return false;
            }
        }
    }
    return false;
}

void Player::removeItem(string name)
{
    list<Item>::iterator lit;
    for (lit = _items.begin(); lit != _items.end(); lit++)
    {
        if ((*lit).getName() == name)
        {
            _items.erase(lit);
            return;
        }
    }
}
void Player::addItemWeapon(Item newItem)
{
    
    _weaponatk = newItem.getAttack();
    _weapon = newItem.getName();
    _items.push_front(newItem);
}

void Player::addItemArmor(Item newItem)
{
    _armordef = newItem.getDefense();
    _armor = newItem.getName();
    _items.push_back(newItem);
}














//Sets the position of the player
void Player::setPosition(int x, int y) {
    _x = x;
    _y = y;
}

void Player::printstats()
{
    int attack = _attack;
    int defense = _defense;
    int health = _health;
    int maxhealth = _maxhealth;
    int level = _level;
    int experience = _experience;
    int maxexperience = _experiencecap;
    int gold = _gold;
    int weaponAtk = _weaponatk;
    int armorDef = _armordef;
    string Weapon = _weapon;
    string Armor = _armor;

    std::cout << "You are level " << level << "\n";
    std::cout << "Atk: " << attack + weaponAtk << "   Def: " << defense + armorDef << "   Hp: " << health << " / " << maxhealth << "\n";
    std::cout << "Exp: " << experience << " / " << maxexperience << "   Gold: " << gold << "\n\n";
    std::cout << "--------Equipment--------\n";
    std::cout << "Weapon: " << Weapon << ", +" << weaponAtk << " Atk\n";
    std::cout << "Armor: " << Armor << ", +" << armorDef << " Def\n";
}

void Player::printInventory()
{
    std::cout << "-------------------------\n";

    list<Item>::iterator lit;

    int i = 0;

    for (lit = _items.begin(); lit != _items.end(); lit++)
    {
        cout << i << ". " << (*lit).getName() << endl;
        i++;
    }
}

void Player::updategear()
{
}


void Player::addExperience(int experience)
{
    _experience += experience;
    _gold += (experience * 2);

    //Level Up!
    while (_experience > _experiencecap)
    {
        std::cout << std::string(100, '\n');

        printf("Leveled up!\n\nYou have gained:\n+1 Atk   +1 Def   +10 Hp\n\n");
        _experience -= _experiencecap;
        _attack += 1;
        _defense += 1;
        _maxhealth += 10;
        _health += 10;
        _level++;
        _experiencecap *= 1.1;

        printstats();
        std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n";

        system("PAUSE");
    }

}

//Gets the position of the player using reference variables
void Player::getPosition(int& x, int& y) {
    x = _x;
    y = _y;
}
