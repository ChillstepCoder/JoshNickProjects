// WheelCollider.cpp
#include "WheelCollider.h"
#include <iostream>

WheelCollider::WheelCollider(b2BodyId carBody, const Config& config)
  : m_carBody(carBody)
  , m_config(config) {
  createCollider();
}

void WheelCollider::createCollider() {
  if (!b2Body_IsValid(m_carBody)) return;

  // Create a rectangular shape for the wheel
  b2Polygon wheelShape = b2MakeBox(m_config.width * 0.5f, m_config.height * 0.5f);

  // Create the shape definition
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.friction = 0.0f;  // We'll handle friction manually
  shapeDef.density = 0.1f;   // Light weight for the wheels
  shapeDef.isSensor = true;  // Make it a sensor so it doesn't affect physics

  // Set the shape's local position relative to car body
  b2Transform localTransform = b2Transform_identity;
  localTransform.p = { m_config.offset.x, m_config.offset.y };

  // Create the shape and store its ID
  m_shapeId = b2CreatePolygonShape(m_carBody, &shapeDef, &wheelShape);
  if (!b2Shape_IsValid(m_shapeId)) {
    std::cerr << "Failed to create wheel collider shape\n";
  }
}

void WheelCollider::update() {
  detectSurface();
}

float WheelCollider::getFrictionMultiplier() const {
  switch (m_currentSurface) {
  case Surface::Road:
    return 1.0f;
  case Surface::RoadOffroad:
    return 0.8f;
  case Surface::Offroad:
    return 0.6f;
  case Surface::OffroadGrass:
    return 0.4f;
  case Surface::Grass:
    return 0.2f;
  default:
    return 1.0f;
  }
}

glm::vec2 WheelCollider::getPosition() const {
  if (!b2Body_IsValid(m_carBody)) return glm::vec2(0.0f);

  // Get car's transform
  b2Transform carTransform = b2Body_GetTransform(m_carBody);

  // Transform wheel's local offset to world position
  b2Vec2 worldPos = b2TransformPoint(carTransform, { m_config.offset.x, m_config.offset.y });
  return glm::vec2(worldPos.x, worldPos.y);
}

float WheelCollider::getAngle() const {
  if (!b2Body_IsValid(m_carBody)) return 0.0f;
  return b2Rot_GetAngle(b2Body_GetRotation(m_carBody));
}

const char* WheelCollider::getSurfaceName(Surface surface) {
  switch (surface) {
  case Surface::Road: return "Road";
  case Surface::RoadOffroad: return "Road/Offroad";
  case Surface::Offroad: return "Offroad";
  case Surface::OffroadGrass: return "Offroad/Grass";
  case Surface::Grass: return "Grass";
  default: return "Unknown";
  }
}

void WheelCollider::detectSurface() {
  if (!b2Body_IsValid(m_carBody)) return;

  // Get the wheel's world position
  b2Transform carTransform = b2Body_GetTransform(m_carBody);
  b2Vec2 wheelPos = b2TransformPoint(carTransform, { m_config.offset.x, m_config.offset.y });

  // Create a small box for overlap testing
  float halfWidth = m_config.width * 0.5f;
  float halfHeight = m_config.height * 0.5f;

  // Get the car's current angle
  float angle = b2Rot_GetAngle(carTransform.q);

  // Create vertices for the wheel box, rotated by the car's angle
  b2Vec2 vertices[4];
  float cosAngle = cosf(angle);
  float sinAngle = sinf(angle);

  vertices[0] = { // Top Left
      wheelPos.x + (-halfWidth * cosAngle - halfHeight * sinAngle),
      wheelPos.y + (-halfWidth * sinAngle + halfHeight * cosAngle)
  };
  vertices[1] = { // Top Right
      wheelPos.x + (halfWidth * cosAngle - halfHeight * sinAngle),
      wheelPos.y + (halfWidth * sinAngle + halfHeight * cosAngle)
  };
  vertices[2] = { // Bottom Right
      wheelPos.x + (halfWidth * cosAngle + halfHeight * sinAngle),
      wheelPos.y + (halfWidth * sinAngle - halfHeight * cosAngle)
  };
  vertices[3] = { // Bottom Left
      wheelPos.x + (-halfWidth * cosAngle + halfHeight * sinAngle),
      wheelPos.y + (-halfWidth * sinAngle - halfHeight * cosAngle)
  };

  // Get world ID
  b2WorldId worldId = b2Body_GetWorld(m_carBody);

  // Track what surfaces we're touching
  bool touchingRoad = false;
  bool touchingOffroad = false;
  bool touchingGrass = false;

  // Create polygon for overlap test
  b2Polygon wheelPoly;
  wheelPoly.count = 4;
  for (int i = 0; i < 4; ++i) {
    wheelPoly.vertices[i] = vertices[i];
  }
  wheelPoly.normals[0] = { cosAngle, sinAngle };
  wheelPoly.normals[1] = { -sinAngle, cosAngle };
  wheelPoly.normals[2] = { -cosAngle, -sinAngle };
  wheelPoly.normals[3] = { sinAngle, -cosAngle };

  // Setup query filter
  b2QueryFilter filter;
  filter.categoryBits = 0xFFFF;  // Match all categories
  filter.maskBits = 0xFFFF;

  // Overlap callback
  auto overlapCallback = [](b2ShapeId shapeId, void* context) -> bool {
    auto* surfaces = static_cast<std::tuple<bool*, bool*, bool*>*>(context);
    b2Filter filter = b2Shape_GetFilter(shapeId);

    // Use shape filter bits to determine surface type
    // Adjust these category bits based on your game's setup
    if (filter.categoryBits & 0x0001) { // Road
      *std::get<0>(*surfaces) = true;
    }
    else if (filter.categoryBits & 0x0002) { // Offroad
      *std::get<1>(*surfaces) = true;
    }
    else if (filter.categoryBits & 0x0004) { // Grass
      *std::get<2>(*surfaces) = true;
    }
    return true;  // Continue checking other shapes
    };

  std::tuple<bool*, bool*, bool*> surfaces(&touchingRoad, &touchingOffroad, &touchingGrass);
  b2World_OverlapPolygon(worldId, &wheelPoly, b2Transform_identity, filter, overlapCallback, &surfaces);

  // Determine surface type based on what we're touching
  if (touchingRoad) {
    if (touchingOffroad) {
      m_currentSurface = Surface::RoadOffroad;
    }
    else {
      m_currentSurface = Surface::Road;
    }
  }
  else if (touchingOffroad) {
    if (touchingGrass) {
      m_currentSurface = Surface::OffroadGrass;
    }
    else {
      m_currentSurface = Surface::Offroad;
    }
  }
  else if (touchingGrass) {
    m_currentSurface = Surface::Grass;
  }
  else {
    m_currentSurface = Surface::Grass;  // Default to grass if not touching anything
  }
}
