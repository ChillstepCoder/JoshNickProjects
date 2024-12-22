// TrafficConeObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class TrafficConeObject : public PlaceableObject {
public:
  TrafficConeObject(const std::string& texturePath, PlacementZone zone)
    : PlaceableObject(texturePath, zone) {
    m_scale = glm::vec2(0.05f);
    m_collisionType = CollisionType::PUSHABLE;
  }

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    float radius = 5.0f; // for some reason changing this does nothing about the traffic cone collision shape.
    std::cout << "Creating traffic cone collision shape with radius: " << radius << std::endl;

    physics->createCircleShape(bodyId, radius,
      CATEGORY_PUSHABLE,
      CATEGORY_CAR | CATEGORY_PUSHABLE | CATEGORY_BARRIER | CATEGORY_SOLID,
      CollisionType::PUSHABLE,
      0.2f,
      0.4f);
  }

  bool isDetectable() const override { return false; }
};
