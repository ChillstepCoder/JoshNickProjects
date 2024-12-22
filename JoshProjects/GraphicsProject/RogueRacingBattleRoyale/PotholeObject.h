// PotHoleObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class PotholeObject : public PlaceableObject {
public:
  PotholeObject(const std::string& texturePath, PlacementZone zone)
    : PlaceableObject(texturePath, zone) {
    m_scale = glm::vec2(0.1f);
    m_collisionType = CollisionType::HAZARD;
  }

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    float radius = 10.0f;
    physics->createCircleShape(bodyId, radius,
      CATEGORY_HAZARD, CATEGORY_HAZARD,
      m_collisionType);
  }

  bool isDetectable() const override { return false; }
};
