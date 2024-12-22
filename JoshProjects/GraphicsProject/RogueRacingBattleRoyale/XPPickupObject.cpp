// XPPickupObject.cpp
#include "XPPickupObject.h"
#include "Car.h"

const XPProperties XPPickupObject::m_defaultXPProps = {
    1,    // xpAmount
    3.0f, // respawnTime
    true, // isActive
    0.0f  // respawnTimer
};

XPPickupObject::XPPickupObject(const std::string& texturePath, PlacementZone zone)
  : PlaceableObject(texturePath, zone) {
  setCollisionType(CollisionType::POWERUP);
  setAutoAlignToTrack(true);
  setScale(glm::vec2(0.1f));

  // Create and initialize XP properties
  m_xpProps = std::make_unique<XPProperties>();
  m_xpProps->xpAmount = 1;
  m_xpProps->respawnTime = 3.0f;
  m_xpProps->isActive = true;
  m_xpProps->respawnTimer = 0.0f;
}

XPPickupObject::XPPickupObject(const XPPickupObject& other) : PlaceableObject(other) {
  if (other.m_xpProps) {
    m_xpProps = std::make_unique<XPProperties>(*other.m_xpProps);
  }
}

void XPPickupObject::onCarCollision(Car* car) {
  std::cout << "XP Pickup onCarCollision called" << std::endl;
  std::cout << "XP Props valid: " << (m_xpProps ? "yes" : "no") << std::endl;
  std::cout << "XP Active: " << (m_xpProps && m_xpProps->isActive ? "yes" : "no") << std::endl;

  if (!car || !m_xpProps || !m_xpProps->isActive) {
    std::cout << "XP Pickup collision skipped - car: " << (car ? "valid" : "null")
      << " props: " << (m_xpProps ? "valid" : "null")
      << " active: " << (m_xpProps && m_xpProps->isActive ? "yes" : "no")
      << std::endl;
    return;
  }

  std::cout << "XP Pickup collision processing - adding " << m_xpProps->xpAmount << " XP" << std::endl;

  // Get current car properties
  auto props = car->getProperties();
  int oldXP = props.totalXP;

  // Increment XP
  props.totalXP += m_xpProps->xpAmount;

  // Update car properties
  car->setProperties(props);

  // Deactivate the XP pickup
  setActive(false);

  std::cout << "XP Pickup complete - XP changed from " << oldXP << " to " << props.totalXP << std::endl;
}

std::unique_ptr<PlaceableObject> XPPickupObject::clone() const {
  auto copy = std::make_unique<XPPickupObject>(*this);
  copy->setActive(true);
  return copy;
}

const XPProperties& XPPickupObject::getXPProperties() const {
  return m_xpProps ? *m_xpProps : m_defaultXPProps;
}

void XPPickupObject::setActive(bool active) {
  if (!m_xpProps) return;
  m_xpProps->isActive = active;
  if (!active) {
    m_xpProps->respawnTimer = m_xpProps->respawnTime;
  }
}

void XPPickupObject::updateRespawnTimer(float deltaTime) {
  if (m_xpProps && !m_xpProps->isActive) {
    m_xpProps->respawnTimer -= deltaTime;
    if (m_xpProps->respawnTimer <= 0.0f) {
      m_xpProps->isActive = true;
      m_xpProps->respawnTimer = 0.0f;
    }
  }
}
