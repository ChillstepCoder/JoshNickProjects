// PlaceableObject.h

#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <JAGEngine/GLTexture.h>
#include <JAGEngine/ResourceManager.h>
#include "PhysicsSystem.h"
#include "ObjectProperties.h"

class Car;

enum class PlacementZone {
  Road,
  Offroad,
  Grass,
  Anywhere
};

class PlaceableObject {
public:
  PlaceableObject(const std::string& texturePath, PlacementZone zone)
    : m_texturePath(texturePath), m_zone(zone), m_physicsBody(b2_nullBodyId),
    m_collisionShape(b2_nullShapeId), m_autoAlignToTrack(false) {
    m_texture = JAGEngine::ResourceManager::getTexture(texturePath);
    m_displayName = texturePath.substr(texturePath.find_last_of("/\\") + 1);
    m_scale = glm::vec2(1.0f);
    m_collisionType = CollisionType::DEFAULT;
  }

  PlaceableObject(const PlaceableObject& other)
    : m_texturePath(other.m_texturePath),
    m_displayName(other.m_displayName),
    m_texture(other.m_texture),
    m_position(other.m_position),
    m_rotation(other.m_rotation),
    m_scale(other.m_scale),
    m_zone(other.m_zone),
    m_isSelected(other.m_isSelected),
    m_physicsBody(b2_nullBodyId),
    m_collisionShape(b2_nullShapeId),
    m_collisionType(other.m_collisionType),
    m_autoAlignToTrack(other.m_autoAlignToTrack) {
  }

  virtual ~PlaceableObject() {
    if (b2Body_IsValid(m_physicsBody)) {
      b2DestroyBody(m_physicsBody);
    }
  }

  virtual std::unique_ptr<PlaceableObject> clone() const {
    // Default implementation creates a copy of the same type
    return std::make_unique<PlaceableObject>(*this);
  }

  virtual void createCollisionShape(b2BodyId bodyId, PhysicsSystem* physics) {
    // Default simple circle shape
    float radius = 7.5f;
    physics->createCircleShape(bodyId, radius,
      CATEGORY_SOLID, CATEGORY_CAR | CATEGORY_PUSHABLE,
      m_collisionType);
  }

  // Virtual functions overridden by derived classes
  // TODO: JOSH: Later, strive to get rid of isBooster and isXPPickup
  // If you NEED to find the object type for some reason, you can use
  // dynamic_cast OR you can make an enum class ObjectType { Booster, XPPickup, Default } and have a getter for that
  virtual bool isBooster() const { return false; }
  virtual bool isXPPickup() const { return false; }
  virtual bool isDetectable() const { return false; }

  virtual const BoosterProperties& getBoosterProperties() const {
    static BoosterProperties defaultProps; return defaultProps;
  }
  virtual const XPProperties& getXPProperties() const {
    static XPProperties defaultProps; return defaultProps;
  }
  virtual void onCarCollision(Car* car) {
    // Empty
  }

  const glm::vec2& getPosition() const { return m_position; }
  float getRotation() const { return m_rotation; }
  const glm::vec2& getScale() const { return m_scale; }
  PlacementZone getZone() const { return m_zone; }
  const JAGEngine::GLTexture& getTexture() const { return m_texture; }
  const std::string& getDisplayName() const { return m_displayName; }
  b2BodyId getPhysicsBody() const { return m_physicsBody; }

  CollisionType getCollisionType() const { return m_collisionType; }
  bool shouldAutoAlignToTrack() const { return m_autoAlignToTrack; }
  bool isSelected() const { return m_isSelected; }

  void setPosition(const glm::vec2& pos) { m_position = pos; }
  void setRotation(float rot) { m_rotation = rot; }
  void setScale(const glm::vec2& scale) { m_scale = scale; }
  void setSelected(bool selected) { m_isSelected = selected; }
  void setAutoAlignToTrack(bool align) { m_autoAlignToTrack = align; }
  void setPhysicsBody(b2BodyId bodyId) { m_physicsBody = bodyId; }
  void setCollisionShape(b2ShapeId shapeId) { m_collisionShape = shapeId; }
  void setCollisionType(CollisionType type) { m_collisionType = type; }

  virtual void setActive(bool active) {}
  virtual void updateRespawnTimer(float deltaTime) {}

  glm::vec4 getBounds() const {
    float width = m_texture.width * m_scale.x;
    float height = m_texture.height * m_scale.y;
    return glm::vec4(m_position.x - width / 2,
      m_position.y - height / 2,
      width,
      height);
  }

protected:
  std::string m_texturePath;
  std::string m_displayName;
  JAGEngine::GLTexture m_texture;
  glm::vec2 m_position = glm::vec2(0.0f);
  float m_rotation = 0.0f;
  glm::vec2 m_scale = glm::vec2(1.0f);
  PlacementZone m_zone;
  bool m_isSelected = false;
  b2BodyId m_physicsBody;
  b2ShapeId m_collisionShape;
  CollisionType m_collisionType;
  bool m_autoAlignToTrack;
};
