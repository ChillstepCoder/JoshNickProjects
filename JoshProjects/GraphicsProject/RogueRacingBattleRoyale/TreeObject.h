// TreeObject.h

#pragma once
#include "PlaceableObject.h"
#include "ObjectProperties.h"
#include <string>

class TreeObject : public PlaceableObject {
public:
  static std::string getDefaultTexturePath() { return "Textures/tree.png"; }
  static PlacementZone getDefaultZone() { return PlacementZone::Grass; }

  TreeObject() : PlaceableObject(getDefaultTexturePath(), getDefaultZone()) {
    m_scale = glm::vec2(0.5f);
    m_collisionType = CollisionType::DEFAULT;
  }

  std::unique_ptr<PlaceableObject> clone() const override {
    //std::cout << "Tree clone called" << std::endl;
    return std::make_unique<TreeObject>(*this);
  }

  void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) override {
    float radius = 10.0f;
    //std::cout << "Creating TREE collision shape with radius: " << radius << std::endl;
    physics->createCircleShape(bodyId, radius,
      CATEGORY_SOLID,
      CATEGORY_CAR | CATEGORY_PUSHABLE,
      CollisionType::DEFAULT,
      1.0f,  // Full density
      0.3f   // Standard friction
    );
  }
  ObjectType getObjectType() const override { return ObjectType::Tree; }
};
