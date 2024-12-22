// BoosterObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"

class Car;

class BoosterObject : public PlaceableObject {
public:
  BoosterObject(const std::string& texturePath, PlacementZone zone);
  BoosterObject(const BoosterObject& other);

  void onCarCollision(Car* car) override;
  std::unique_ptr<PlaceableObject> clone() const override;
  bool isBooster() const override { return true; }
  bool isDetectable() const override { return true; }
  const BoosterProperties& getBoosterProperties() const override;

private:
  std::unique_ptr<BoosterProperties> m_boosterProps;
  static const BoosterProperties m_defaultBoosterProps;
};
