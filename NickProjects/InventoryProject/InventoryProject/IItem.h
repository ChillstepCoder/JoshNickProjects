#pragma once
#include <iostream>
#include <string>
#include <vector>

////////////ABSTRACT CLASS////////////

class IItem {
public:
    friend class Inventory;

    IItem(std::string Name, int Value, float Weight);
    virtual ~IItem() = default;

    std::string getName() const { return Name; }
    int getValue() const { return Value; }
    float getWeight() const { return Weight; }

    void setName(); //sets name of item
    void setValue(); //sets value of item
    void setWeight(); //sets weight of item

    virtual void print(); //virtual functon (prints name of item)

    virtual void use() = 0; //pure virtual function

protected:
    std::string Name = "";
    int Value = 0; //Gold
    float Weight = 0.0f; //Lbs
};


//////////POLYMORPHIC CLASSES/////////

//Weapon//////////////////////////////
class Weapon : public IItem {
public:
    enum class WeaponType { Sword, LongSword, ShortSword };

    Weapon(std::string Name, int Value, float Weight, WeaponType type);

    void print(); //prints name + custom info
    void use(); //equips weapon

    WeaponType getWeaponType() const { return _type; }
    std::string getWeaponTypeName(WeaponType type) const;

private:
    WeaponType _type;
};

//Armor///////////////////////////////

class Armor : public IItem {
public:

    Armor(std::string Name, int Value, float Weight);

    void print(); //prints name + custom info

    void use(); //equips armor

private:

};

//Potion//////////////////////////////

class Potion : public IItem {
public:

    enum class PotionSize { Small, Medium, Large, Grand };

    Potion(std::string Name, int Value, float Weight, int Charges, PotionSize size);

    void print(); //prints name + custom info

    void use(); //drinks potion, charges -1.

    void setCharges(int c) { _Charges = c; } //setter

    int getCharges() const { return _Charges; } //getter

    PotionSize getPotionSize() const { return _size; }//getter
    std::string getPotionSizeName(PotionSize size) const;

private:
    int _Charges; //amount of times you can drink
    PotionSize _size;
};

//Scroll//////////////////////////////

class Scroll : public IItem {
public:

    Scroll(std::string Name, int Value, float Weight, int Charges);

    void print(); //prints name + custom info

    void use(); //casts scroll

    void setCharges(int c) { _Charges = c; } //setter

    int getCharges() const { return _Charges; } //getter

private:
    int _Charges;//amount of times you can use scroll. ususally 1
};



