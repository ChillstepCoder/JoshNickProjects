// TreeObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class TreeObject : public PlaceableObject {
public:
  TreeObject(const std::string& texturePath, PlacementZone zone)
    : PlaceableObject(texturePath, zone) {
    // Original logic: trees had scale=0.5f, collision=DEFAULT
    m_scale = glm::vec2(0.5f);
    m_collisionType = CollisionType::DEFAULT;
  }

  bool isDetectable() const override { return false; }
};
