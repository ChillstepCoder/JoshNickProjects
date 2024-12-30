// PotholeObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class PotholeObject : public PlaceableObject {
public:
  static std::string getDefaultTexturePath() { return "Textures/pothole.png"; }
  static PlacementZone getDefaultZone() { return PlacementZone::Road; }

  PotholeObject() : PlaceableObject(getDefaultTexturePath(), getDefaultZone()) {
    m_scale = glm::vec2(0.1f);
    m_collisionType = CollisionType::HAZARD;
  }

  std::unique_ptr<PlaceableObject> clone() const override {
    std::cout << "Pothole clone called" << std::endl;
    return std::make_unique<PotholeObject>(*this);
  }

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    std::cout << "\n=== Pothole createCollisionShape Called ===\n";
    std::cout << "Pothole scale: " << m_scale.x << ", " << m_scale.y << std::endl;
    std::cout << "Pothole collision type: " << static_cast<int>(m_collisionType) << std::endl;
    float radius = 8.5f;
    physics->createCircleShape(bodyId, radius,
      CATEGORY_HAZARD,
      CATEGORY_HAZARD,
      m_collisionType);
    std::cout << "=== End Pothole Shape Creation ===\n";
  }
  ObjectType getObjectType() const override { return ObjectType::Pothole; }
};
