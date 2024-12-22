// BoosterObject.cpp

#include "BoosterObject.h"
#include "Car.h"

const BoosterProperties BoosterObject::m_defaultBoosterProps = {
    1500.0f,  // maxBoostSpeed
    100.0f,   // boostAccelRate
    0.95f,    // boostDecayRate
    1.0f      // directionFactor
};


BoosterObject::BoosterObject(const std::string& texturePath, PlacementZone zone)
  : PlaceableObject(texturePath, zone) {
  setCollisionType(CollisionType::POWERUP);
  setAutoAlignToTrack(true);
  setScale(glm::vec2(0.15f));
  m_boosterProps = std::make_unique<BoosterProperties>();
}

BoosterObject::BoosterObject(const BoosterObject& other) : PlaceableObject(other) {
  if (other.m_boosterProps) {
    m_boosterProps = std::make_unique<BoosterProperties>(*other.m_boosterProps);
  }
}

void BoosterObject::onCarCollision(Car* car) {
  if (!car || !m_boosterProps) return;
  car->onSensorEnter(getPhysicsBody());
}

std::unique_ptr<PlaceableObject> BoosterObject::clone() const {
  return std::make_unique<BoosterObject>(*this);
}

const BoosterProperties& BoosterObject::getBoosterProperties() const {
  return m_boosterProps ? *m_boosterProps : m_defaultBoosterProps;
}
