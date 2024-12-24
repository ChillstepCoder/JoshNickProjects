// TrafficConeObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class TrafficConeObject : public PlaceableObject {
public:
  static std::string getDefaultTexturePath() { return "Textures/traffic_cone.png"; }
  static PlacementZone getDefaultZone() { return PlacementZone::Anywhere; }

  TrafficConeObject() : PlaceableObject(getDefaultTexturePath(), getDefaultZone()) {
    m_scale = glm::vec2(0.05f);
    m_collisionType = CollisionType::PUSHABLE;
  }

  std::unique_ptr<PlaceableObject> clone() const override {
    std::cout << "TrafficCone clone called" << std::endl;
    return std::make_unique<TrafficConeObject>(*this);
  }

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    std::cout << "\n=== TrafficCone createCollisionShape Called ===\n";
    std::cout << "Cone scale: " << m_scale.x << ", " << m_scale.y << std::endl;
    std::cout << "Cone collision type: " << static_cast<int>(m_collisionType) << std::endl;
    float radius = 5.0f;
    physics->createCircleShape(bodyId, radius,
      CATEGORY_PUSHABLE,
      CATEGORY_CAR | CATEGORY_PUSHABLE | CATEGORY_BARRIER | CATEGORY_SOLID,
      CollisionType::PUSHABLE,
      0.2f,
      0.4f);
    std::cout << "=== End TrafficCone Shape Creation ===\n";
  }
  bool isDetectable() const override { return true; }
};
