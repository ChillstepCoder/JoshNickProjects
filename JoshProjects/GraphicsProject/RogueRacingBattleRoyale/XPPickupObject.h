// XPPickupObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <iostream>

class Car;

class XPPickupObject : public PlaceableObject {
public:
  XPPickupObject(const std::string& texturePath, PlacementZone zone);
  XPPickupObject(const XPPickupObject& other);

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    float radius = 10.0f; //10 is perfect
    std::cout << "Creating XP star collision shape with radius: " << radius << std::endl;

    physics->createCircleShape(bodyId, radius,
      CATEGORY_POWERUP,
      CATEGORY_CAR,
      CollisionType::POWERUP);
  }

  void onCarCollision(Car* car) override;
  std::unique_ptr<PlaceableObject> clone() const override;
  bool isXPPickup() const override { return true; }
  bool isDetectable() const override { return true; }
  const XPProperties& getXPProperties() const override;
  void setActive(bool active) override;
  void updateRespawnTimer(float deltaTime) override;

private:
  std::unique_ptr<XPProperties> m_xpProps;
  static const XPProperties m_defaultXPProps;
};
