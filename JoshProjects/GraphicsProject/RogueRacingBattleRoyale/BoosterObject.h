// BoosterObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <iostream>

class Car;
class BoosterObject : public PlaceableObject {
public:
  static std::string getDefaultTexturePath() { return "Textures/booster.png"; }
  static PlacementZone getDefaultZone() { return PlacementZone::Road; }

  BoosterObject() : PlaceableObject(getDefaultTexturePath(), getDefaultZone()) {
    m_scale = glm::vec2(0.15f);
    m_collisionType = CollisionType::POWERUP;
    m_boosterProps = std::make_unique<BoosterProperties>();
    m_autoAlignToTrack = true;
  }

  BoosterObject(const BoosterObject& other);

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    float width = 48.0f;
    float height = 30.0f;
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
