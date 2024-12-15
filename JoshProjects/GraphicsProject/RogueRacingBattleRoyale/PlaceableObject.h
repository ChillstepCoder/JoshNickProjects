// PlaceableObject.h

#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <JAGEngine/GLTexture.h>
#include <JAGEngine/ResourceManager.h>
#include "PhysicsSystem.h"

enum class PlacementZone {
  Road,
  Offroad,
  Grass,
  Anywhere
};


class PlaceableObject {
public:
  struct BoosterProperties {
    float maxBoostSpeed = 1500.0f;     // Maximum speed boost
    float boostAccelRate = 100.0f;     // How quickly boost builds up per frame
    float boostDecayRate = 0.95f;      // How quickly boost decays when off the pad
    float directionFactor = 1.0f;      // How much the approach angle affects boost
  };

  PlaceableObject(const std::string& texturePath, PlacementZone zone)
    : m_texturePath(texturePath)
    , m_zone(zone)
    , m_physicsBody(b2_nullBodyId)
    , m_collisionShape(b2_nullShapeId)
    , m_autoAlignToTrack(false) {

    m_texture = JAGEngine::ResourceManager::getTexture(texturePath);

    size_t lastSlash = texturePath.find_last_of("/\\");
    m_displayName = (lastSlash != std::string::npos) ?
      texturePath.substr(lastSlash + 1) : texturePath;

    // Set default properties based on object type
    if (m_displayName.find("pothole") != std::string::npos) {
      m_scale = glm::vec2(0.1f);
      m_collisionType = PhysicsSystem::CollisionType::HAZARD;
    }
    else if (m_displayName.find("tree") != std::string::npos) {
      m_scale = glm::vec2(0.5f);
      m_collisionType = PhysicsSystem::CollisionType::DEFAULT;
    }
    else if (m_displayName.find("cone") != std::string::npos) {
      m_scale = glm::vec2(0.05f);
      m_collisionType = PhysicsSystem::CollisionType::PUSHABLE;
    }
    else if (m_displayName.find("booster") != std::string::npos) {
      m_scale = glm::vec2(0.15f);
      m_collisionType = PhysicsSystem::CollisionType::POWERUP;
      m_autoAlignToTrack = true;

      // Initialize booster properties
      m_boosterProps = std::make_unique<BoosterProperties>();
    }
    else {
      m_scale = glm::vec2(1.0f);
      m_collisionType = PhysicsSystem::CollisionType::DEFAULT;
    }
  }

  // Copy constructor
  PlaceableObject(const PlaceableObject& other)
    : m_texturePath(other.m_texturePath)
    , m_displayName(other.m_displayName)
    , m_texture(other.m_texture)
    , m_position(other.m_position)
    , m_rotation(other.m_rotation)
    , m_scale(other.m_scale)
    , m_zone(other.m_zone)
    , m_isSelected(other.m_isSelected)
    , m_physicsBody(b2_nullBodyId)
    , m_collisionShape(b2_nullShapeId)
    , m_collisionType(other.m_collisionType)
    , m_autoAlignToTrack(other.m_autoAlignToTrack) {
  }

  ~PlaceableObject() {
    if (b2Body_IsValid(m_physicsBody)) {
      b2DestroyBody(m_physicsBody);
      m_physicsBody = b2_nullBodyId;
    }
  }

  // Getters
  bool isBooster() const {
    return m_collisionType == PhysicsSystem::CollisionType::POWERUP &&
      m_displayName.find("booster") != std::string::npos;
  }
  const BoosterProperties& getBoosterProperties() const {
    static const BoosterProperties defaultProps;
    return m_boosterProps ? *m_boosterProps : defaultProps;
  }
  const glm::vec2& getPosition() const { return m_position; }
  float getRotation() const { return m_rotation; }
  const glm::vec2& getScale() const { return m_scale; }
  PlacementZone getZone() const { return m_zone; }
  const JAGEngine::GLTexture& getTexture() const { return m_texture; }
  const std::string& getDisplayName() const { return m_displayName; }
  b2BodyId getPhysicsBody() const { return m_physicsBody; }
  PhysicsSystem::CollisionType getCollisionType() const { return m_collisionType; }
  bool shouldAutoAlignToTrack() const { return m_autoAlignToTrack; }

  // Setters
  void setPosition(const glm::vec2& pos) { m_position = pos; }
  void setRotation(float rot) { m_rotation = rot; }
  void setScale(const glm::vec2& scale) { m_scale = scale; }
  void setSelected(bool selected) { m_isSelected = selected; }
  bool isSelected() const { return m_isSelected; }
  void setAutoAlignToTrack(bool align) { m_autoAlignToTrack = align; }
  void setPhysicsBody(b2BodyId bodyId) { m_physicsBody = bodyId; }
  void setCollisionShape(b2ShapeId shapeId) { m_collisionShape = shapeId; }

  // Get object bounds for selection
  glm::vec4 getBounds() const {
    float width = m_texture.width * m_scale.x;
    float height = m_texture.height * m_scale.y;
    return glm::vec4(
      m_position.x - width / 2,
      m_position.y - height / 2,
      width,
      height
    );
  }

private:
  std::string m_texturePath;
  std::string m_displayName;
  JAGEngine::GLTexture m_texture;
  glm::vec2 m_position = glm::vec2(0.0f);
  float m_rotation = 0.0f;
  glm::vec2 m_scale = glm::vec2(1.0f);
  PlacementZone m_zone;
  bool m_isSelected = false;

  b2BodyId m_physicsBody = b2_nullBodyId;
  b2ShapeId m_collisionShape = b2_nullShapeId;
  PhysicsSystem::CollisionType m_collisionType;

  float m_drawOrder;
  bool m_autoAlignToTrack;

  std::unique_ptr<BoosterProperties> m_boosterProps;
};
