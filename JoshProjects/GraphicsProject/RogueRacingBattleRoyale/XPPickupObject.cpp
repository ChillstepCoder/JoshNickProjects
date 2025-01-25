// XPPickupObject.cpp

#include "XPPickupObject.h"
#include "Car.h"

const XPProperties XPPickupObject::m_defaultXPProps = {
    1,    // xpAmount
    3.0f, // respawnTime
    true, // isActive
    0.0f  // respawnTimer
};

// Remove old constructor, only keep copy constructor
XPPickupObject::XPPickupObject(const XPPickupObject& other)
  : PlaceableObject(other) {
  if (other.m_xpProps) {
    m_xpProps = std::make_unique<XPProperties>(*other.m_xpProps);
  }
}

void XPPickupObject::onCarCollision(Car* car) {
  //std::cout << "XP Pickup onCarCollision called" << std::endl;

  // Add an early exit check using atomic bool
  static std::atomic<bool> isProcessingCollision{ false };

  // If we're already processing a collision, exit early
  if (isProcessingCollision.exchange(true)) {
   // std::cout << "Collision already being processed, skipping" << std::endl;
    return;
  }

  if (!car || !m_xpProps || !m_xpProps->isActive) {
   // std::cout << "XP Pickup collision skipped - car: " << (car ? "valid" : "null")
   //   << " props: " << (m_xpProps ? "valid" : "null")
   //   << " active: " << (m_xpProps && m_xpProps->isActive ? "yes" : "no")
    //  << std::endl;
    isProcessingCollision = false;
    return;
  }

  //std::cout << "XP Pickup collision processing - adding " << m_xpProps->xpAmount << " XP" << std::endl;

  auto props = car->getProperties();
  int oldXP = props.totalXP;
  props.totalXP += m_xpProps->xpAmount;
  car->setProperties(props);
  setActive(false);

  //std::cout << "XP Pickup complete - XP changed from " << oldXP << " to " << props.totalXP << std::endl;

  // Reset the processing flag
  isProcessingCollision = false;
}

void XPPickupObject::onEndCollision(Car* car) {
// do nothing
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
  if (!m_xpProps) return;

  if (!m_xpProps->isActive && m_xpProps->respawnTimer > 0.0f) {
    m_xpProps->respawnTimer -= deltaTime;

    // Check if it's time to respawn
    if (m_xpProps->respawnTimer <= 0.0f) {
      m_xpProps->isActive = true;
      m_xpProps->respawnTimer = 0.0f;
      std::cout << "XP Pickup respawned" << std::endl;
    }
  }
}
