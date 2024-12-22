// BoosterObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <iostream>`

class Car;

class BoosterObject : public PlaceableObject {
public:
  BoosterObject(const std::string& texturePath, PlacementZone zone);
  BoosterObject(const BoosterObject& other);

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    // Use EXACT dimensions from old working code
    float width = 48.0f;   // Width of pill 
    float height = 30.0f;  // Height of pill
    std::cout << "Creating booster collision shape with raw dimensions: " << width << "x" << height << std::endl;

    physics->createPillShape(bodyId, width, height,
      CATEGORY_POWERUP,
      CATEGORY_CAR,
      CollisionType::POWERUP);
  }

  void onCarCollision(Car* car) override;
  std::unique_ptr<PlaceableObject> clone() const override;
  bool isBooster() const override { return true; }
  bool isDetectable() const override { return true; }
  const BoosterProperties& getBoosterProperties() const override;

private:
  std::unique_ptr<BoosterProperties> m_boosterProps;
  static const BoosterProperties m_defaultBoosterProps;
};
