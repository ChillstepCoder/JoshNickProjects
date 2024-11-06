// PlaceableObject.h

#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <JAGEngine/GLTexture.h>
#include <JAGEngine/ResourceManager.h>

enum class PlacementZone {
  Road,
  Offroad,
  Grass,
  Anywhere
};

class PlaceableObject {
public:
  PlaceableObject(const std::string& texturePath, PlacementZone zone)
    : m_texturePath(texturePath)
    , m_zone(zone) {
    m_texture = JAGEngine::ResourceManager::getTexture(texturePath);

    // Extract just the filename for display
    size_t lastSlash = texturePath.find_last_of("/\\");
    m_displayName = (lastSlash != std::string::npos) ?
      texturePath.substr(lastSlash + 1) : texturePath;

    // Set default scale based on texture type
    if (m_displayName.find("pothole") != std::string::npos) {
      m_scale = glm::vec2(0.1f);
    }
    else if (m_displayName.find("tree") != std::string::npos) {
      m_scale = glm::vec2(0.5f);
    }
    else if (m_displayName.find("cone") != std::string::npos) {
      m_scale = glm::vec2(0.05f);
    }
    else {
      m_scale = glm::vec2(1.0f);
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
    , m_isSelected(other.m_isSelected) {
  }

  virtual ~PlaceableObject() = default;

  // Getters
  const glm::vec2& getPosition() const { return m_position; }
  float getRotation() const { return m_rotation; }
  const glm::vec2& getScale() const { return m_scale; }
  PlacementZone getZone() const { return m_zone; }
  const JAGEngine::GLTexture& getTexture() const { return m_texture; }
  const std::string& getDisplayName() const { return m_displayName; }

  // Setters
  void setPosition(const glm::vec2& pos) { m_position = pos; }
  void setRotation(float rot) { m_rotation = rot; }
  void setScale(const glm::vec2& scale) { m_scale = scale; }
  void setSelected(bool selected) { m_isSelected = selected; }
  bool isSelected() const { return m_isSelected; }

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
};
