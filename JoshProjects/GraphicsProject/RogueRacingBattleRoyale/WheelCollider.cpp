// WheelCollider.cpp
#include "WheelCollider.h"
#include "Car.h"  // Add this line so that Car is defined

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <iostream>
#include "SplineTrack.h"


WheelCollider::WheelCollider(b2BodyId carBody, const Config& config)
  : m_carBody(carBody)
  , m_config(config) {
  createCollider();
}

void WheelCollider::createCollider() {
  if (!b2Body_IsValid(m_carBody)) return;

  // Create a rectangular shape for the wheel
  b2Polygon wheelShape = b2MakeBox(m_config.height * 0.5f, m_config.width * 0.5f);

  // Create the shape definition
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.friction = 0.3f;
  shapeDef.density = 0.1f;
  shapeDef.isSensor = false;

  // Store the shape and its ID
  m_shapeId = b2CreatePolygonShape(m_carBody, &shapeDef, &wheelShape);
  if (!b2Shape_IsValid(m_shapeId)) {
    std::cerr << "Failed to create wheel collider shape\n";
  }
}
void WheelCollider::update() {
  if (!b2Body_IsValid(m_carBody)) return;
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
  float angle = b2Rot_GetAngle(carTransform.q);

  // Calculate rotation using standard trig functions
  float cos_a = std::cos(angle);
  float sin_a = std::sin(angle);

  // First rotate the offset
  float rotatedX = m_config.offset.x * cos_a - m_config.offset.y * sin_a;
  float rotatedY = m_config.offset.x * sin_a + m_config.offset.y * cos_a;

  // Then add car's position
  float worldX = carTransform.p.x + rotatedX;
  float worldY = carTransform.p.y + rotatedY;

  return glm::vec2(worldX, worldY);
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

  glm::vec2 wheelPos = getPosition();

  // Instead of directly casting to SplineTrack, get the Car first
  Car* car = static_cast<Car*>(b2Body_GetUserData(m_carBody));
  if (!car || !car->getTrack()) {
    m_currentSurface = Surface::Grass;
    return;
  }

  SplineTrack* track = car->getTrack();

  auto splinePoints = track->getSplinePoints(50);
  if (splinePoints.empty()) {
    m_currentSurface = Surface::Grass;
    return;
  }

  // Find closest spline point
  float minDist = std::numeric_limits<float>::max();
  size_t closestIndex = 0;

  for (size_t i = 0; i < splinePoints.size(); ++i) {
    float dist = glm::distance2(wheelPos, splinePoints[i].position);
    if (dist < minDist) {
      minDist = dist;
      closestIndex = i;
    }
  }

  // Get previous and next points
  size_t prevIndex = (closestIndex > 0) ? closestIndex - 1 : splinePoints.size() - 1;
  size_t nextIndex = (closestIndex + 1) % splinePoints.size();

  // Calculate track direction
  glm::vec2 dir1 = glm::normalize(splinePoints[closestIndex].position - splinePoints[prevIndex].position);
  glm::vec2 dir2 = glm::normalize(splinePoints[nextIndex].position - splinePoints[closestIndex].position);
  glm::vec2 trackDir = glm::normalize(dir1 + dir2);
  glm::vec2 trackNormal(-trackDir.y, trackDir.x);

  // Calculate distances
  glm::vec2 toWheel = wheelPos - splinePoints[closestIndex].position;
  float distanceFromCenter = glm::dot(toWheel, trackNormal);
  float absDistance = std::abs(distanceFromCenter);

  // Get road properties
  const auto& point = splinePoints[closestIndex];
  float offroadWidth = (distanceFromCenter > 0) ? point.offroadWidth.x : point.offroadWidth.y;

  // Adjust transition zones
  const float roadTransitionZone = 1.0f;     // Zone between road and offroad
  const float offroadTransitionZone = 1.5f;  // Zone between offroad and grass

  float roadEdge = point.roadWidth;
  float offroadEdge = roadEdge + offroadWidth;

  // Refined surface detection with better transitions
  if (absDistance <= roadEdge) {
    // Fully on road
    m_currentSurface = Surface::Road;
  }
  else if (absDistance <= roadEdge + roadTransitionZone) {
    // In transition between road and offroad
    m_currentSurface = Surface::RoadOffroad;
  }
  else if (absDistance <= offroadEdge - offroadTransitionZone) {
    // Fully in offroad area
    m_currentSurface = Surface::Offroad;
  }
  else if (absDistance <= offroadEdge + offroadTransitionZone) {
    // In transition between offroad and grass
    m_currentSurface = Surface::OffroadGrass;
  }
  else {
    // Fully on grass
    m_currentSurface = Surface::Grass;
  }
}

