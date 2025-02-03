// XPPickupObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <iostream>

class Car;
class XPPickupObject : public PlaceableObject {
public:
  static std::string getDefaultTexturePath() { return "Textures/xpstar.png"; }
  static PlacementZone getDefaultZone() { return PlacementZone::Road; }

  XPPickupObject() : PlaceableObject(getDefaultTexturePath(), getDefaultZone()) {
    m_scale = glm::vec2(0.1f);
    m_collisionType = CollisionType::POWERUP;
    m_xpProps = std::make_unique<XPProperties>();
  }

  XPPickupObject(const XPPickupObject& other);

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    //std::cout << "Creating XP pickup collision shape" << std::endl;
    float radius = 10.0f;
    auto shapeId = physics->createCircleShape(bodyId, radius,
      CATEGORY_POWERUP,
      CATEGORY_CAR,
      CollisionType::POWERUP);
    if (!b2Shape_IsValid(shapeId)) {
      std::cerr << "Failed to create XP pickup shape!" << std::endl;
    }
  }

  bool m_pendingDeactivation = false;

  void onCarCollision(Car* car) override;
  void onEndCollision(Car* car) override;

  std::unique_ptr<PlaceableObject> clone() const override;
  ObjectType getObjectType() const override { return ObjectType::XPPickup; }
  const XPProperties& getXPProperties() const override;
  void setActive(bool active) override;
  void updateRespawnTimer(float deltaTime) override;

private:
  std::unique_ptr<XPProperties> m_xpProps;
  static const XPProperties m_defaultXPProps;
};
