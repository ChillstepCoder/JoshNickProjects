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
  m_xpProps = std::make_unique<XPProperties>();
}

XPPickupObject::XPPickupObject(const XPPickupObject& other) : PlaceableObject(other) {
  if (other.m_xpProps) {
    m_xpProps = std::make_unique<XPProperties>(*other.m_xpProps);
  }
}

void XPPickupObject::onCarCollision(Car* car) {
  if (!car || !m_xpProps || !m_xpProps->isActive) return;
  car->onSensorEnter(getPhysicsBody());
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
