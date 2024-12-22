// TrafficConeObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class TrafficConeObject : public PlaceableObject {
public:
  TrafficConeObject(const std::string& texturePath, PlacementZone zone)
    : PlaceableObject(texturePath, zone) {
    // Original logic: cones scale=0.05f, collision=PUSHABLE
    m_scale = glm::vec2(0.05f);
    m_collisionType = CollisionType::PUSHABLE;
  }

  bool isDetectable() const override { return false; }
};
