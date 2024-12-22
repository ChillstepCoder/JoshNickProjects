// XPPickupObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"

// Forward declare Car
class Car;

class XPPickupObject : public PlaceableObject {
public:
  XPPickupObject(const std::string& texturePath, PlacementZone zone);
  XPPickupObject(const XPPickupObject& other);

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
