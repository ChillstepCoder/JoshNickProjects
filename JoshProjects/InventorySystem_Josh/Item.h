#pragma once

#include <string>

enum class ItemType {
  Weapon,
  Armor,
  Potion,
  Scroll
};

class Item {
protected:
  std::string name;
  int value;
  float weight;

public:
  Item(const std::string& name, int value, float weight);
  virtual ~Item() = default;

  std::string getName() const;
  int getValue() const;
  float getWeight() const;

  void setName(const std::string& newName);
  void setValue(int newValue);
  void setWeight(float newWeight);

  virtual void printQuantity() const;
  virtual bool use() = 0;
  virtual ItemType getType() const = 0;
};
