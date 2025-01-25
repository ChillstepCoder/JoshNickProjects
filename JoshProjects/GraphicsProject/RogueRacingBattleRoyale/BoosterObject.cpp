// BoosterObject.cpp

#include "BoosterObject.h"
#include "Car.h"

const BoosterProperties BoosterObject::m_defaultBoosterProps = {
    1500.0f,  // maxBoostSpeed
    100.0f,   // boostAccelRate
    0.95f,    // boostDecayRate
    1.0f      // directionFactor
};

BoosterObject::BoosterObject(const BoosterObject& other)
  : PlaceableObject(other) {
  if (other.m_boosterProps) {
    m_boosterProps = std::make_unique<BoosterProperties>(*other.m_boosterProps);
  }
}

void BoosterObject::onCarCollision(Car* car) {
  if (!car || !m_boosterProps) return;

  //std::cout << "Booster collision begin with car" << std::endl;

  // Set the car's booster state
  auto props = car->getProperties();
  props.isOnBooster = true;
  props.currentBooster = this;
  car->setProperties(props);
}

void BoosterObject::onEndCollision(Car* car) {
  if (!car) return;

  //std::cout << "Booster collision end with car" << std::endl;

  auto props = car->getProperties();
  props.isOnBooster = false;
  props.currentBooster = nullptr;
  props.boostAccumulator = props.currentBoostSpeed;
  car->setProperties(props);
}

std::unique_ptr<PlaceableObject> BoosterObject::clone() const {
  //std::cout << "Booster clone called" << std::endl;
  return std::make_unique<BoosterObject>(*this);
}

const BoosterProperties& BoosterObject::getBoosterProperties() const {
  return m_boosterProps ? *m_boosterProps : m_defaultBoosterProps;
}
