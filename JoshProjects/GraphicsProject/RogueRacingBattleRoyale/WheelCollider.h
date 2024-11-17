// WheelCollider.h
#pragma once

#include <string>
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <array>

class WheelCollider {
public:
  enum class Surface {
    Road,
    RoadOffroad,    // On the edge between road and offroad
    Offroad,
    OffroadGrass,   // On the edge between offroad and grass
    Grass
  };

  struct Config {
    float width = 6.0f;   // Width of wheel collider
    float height = 3.0f;  // Height of wheel collider
    glm::vec2 offset;     // Offset from car center
  };

  WheelCollider(b2BodyId carBody, const Config& config);
  ~WheelCollider() = default;

  void update();
  Surface getSurface() const { return m_currentSurface; }
  float getFrictionMultiplier() const;

  // Debug helpers
  glm::vec2 getPosition() const;
  float getAngle() const;
  const Config& getConfig() const { return m_config; }

  static const char* getSurfaceName(Surface surface);

private:
  b2BodyId m_carBody;
  Config m_config;
  Surface m_currentSurface = Surface::Road;
  b2ShapeId m_shapeId;

  void detectSurface();
  void createCollider();
};
