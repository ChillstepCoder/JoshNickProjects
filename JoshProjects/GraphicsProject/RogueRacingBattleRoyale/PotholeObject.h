// PotHoleObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class PotholeObject : public PlaceableObject {
public:
  PotholeObject(const std::string& texturePath, PlacementZone zone)
    : PlaceableObject(texturePath, zone) {
    // Set properties based on old code for potholes
    // Original logic: scale=0.1f, collision = HAZARD
    m_scale = glm::vec2(0.1f);
    m_collisionType = CollisionType::HAZARD;
    // Potholes are not powerups or anything special
    // No booster or XP props
  }

  bool isDetectable() const override { return false; } // Potholes may not need detection
  // If you want them to be detectable, return true.
};
