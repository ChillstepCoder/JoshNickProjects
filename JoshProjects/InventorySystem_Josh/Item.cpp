#include "Item.h"
#include <iostream>

Item::Item(const std::string& name, int value, float weight)
  : name(name), value(value), weight(weight) {}

std::string Item::getName() const {
  return name;
}

int Item::getValue() const {
  return value;
}

float Item::getWeight() const {
  return weight;
}

void Item::setName(const std::string& newName) {
  name = newName;
}

void Item::setValue(int newValue) {
  value = newValue;
}

void Item::setWeight(float newWeight) {
  weight = newWeight;
}

void Item::printQuantity() const {
  std::cout << "Name: " << name << ", Value: " << value << " gold, Weight: " << weight << " lbs";
}
