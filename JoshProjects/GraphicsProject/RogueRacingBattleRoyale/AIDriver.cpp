// AIDriver.cpp

#include "AIDriver.h"
#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>

AIDriver::AIDriver(Car* car)
  : m_car(car)
  , m_lastInputTime(0.0f)
  , m_desiredAngle(0.0f)
{
  m_currentInput = InputState();
}

void AIDriver::update(float deltaTime) {
  if (!m_car || !m_car->getTrack()) return;

  // Update target points considering track direction
  m_targetPoint = findClosestSplinePoint();
  m_lookAheadPoint = calculateLookAheadPoint();

  // Only update steering based on reaction time
  m_lastInputTime += deltaTime;
  if (m_lastInputTime >= m_config.reactionTime) {
    m_lastInputTime = 0.0f;
    updateSteering();
  }

  // Always update throttle every frame
  updateThrottle();

  // Apply inputs to car
  m_car->update(m_currentInput);
}

void AIDriver::updateSteering() {
  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  float carAngle = debugInfo.angle;

  // Calculate angle to target point
  glm::vec2 toTarget = m_targetPoint - carPos;
  float targetAngle = atan2(toTarget.y, toTarget.x);

  // Calculate angle to lookahead point
  glm::vec2 toLookAhead = m_lookAheadPoint - carPos;
  float lookAheadAngle = atan2(toLookAhead.y, toLookAhead.x);

  // Blend between current track angle and lookahead angle based on configuration
  m_desiredAngle = targetAngle * (1.0f - m_config.turnAnticipation) +
    lookAheadAngle * m_config.turnAnticipation;

  // Calculate angle difference (-π to π)
  float angleDiff = m_desiredAngle - carAngle;
  while (angleDiff > glm::pi<float>()) angleDiff -= 2.0f * glm::pi<float>();
  while (angleDiff < -glm::pi<float>()) angleDiff += 2.0f * glm::pi<float>();

  // Calculate centering force
  glm::vec2 toSpline = m_targetPoint - carPos;
  float distanceToSpline = glm::length(toSpline);
  float centeringMultiplier = std::min(1.0f, distanceToSpline / 30.0f) * m_config.centeringForce;

  // Apply steering based on angle difference and centering force
  m_currentInput.turningLeft = angleDiff > 0.05f * centeringMultiplier;
  m_currentInput.turningRight = angleDiff < -0.05f * centeringMultiplier;
}

void AIDriver::updateThrottle() {
  auto debugInfo = m_car->getDebugInfo();
  float currentSpeed = debugInfo.currentSpeed;

  glm::vec2 carPos(debugInfo.position);
  glm::vec2 toAhead = glm::normalize(m_lookAheadPoint - carPos);
  glm::vec2 carDir = glm::vec2(cos(debugInfo.angle), sin(debugInfo.angle));
  float turnAngle = acos(glm::dot(toAhead, carDir));

  // Drift initiation threshold
  const float DRIFT_SPEED_THRESHOLD = 200.0f;
  const float DRIFT_ANGLE_THRESHOLD = 0.4f; // About 23 degrees

  // Check if we should initiate a drift
  bool shouldDrift = currentSpeed > DRIFT_SPEED_THRESHOLD && turnAngle > DRIFT_ANGLE_THRESHOLD;

  if (shouldDrift) {
    // Initiate drift by braking briefly and turning
    m_currentInput.braking = true;
    m_currentInput.accelerating = false;

    // Turn harder during drift
    if (turnAngle > 0) {
      m_currentInput.turningLeft = true;
      m_currentInput.turningRight = false;
    }
    else {
      m_currentInput.turningLeft = false;
      m_currentInput.turningRight = true;
    }
  }
  else if (turnAngle < 0.2f) {  // Straight section
    m_currentInput.accelerating = true;
    m_currentInput.braking = false;
  }
  else {
    // Normal corner handling
    float cornerSpeed = calculateCornerSpeed(m_lookAheadPoint);

    if (currentSpeed > cornerSpeed + 25.0f) {
      m_currentInput.braking = true;
      m_currentInput.accelerating = false;
    }
    else {
      m_currentInput.accelerating = true;
      m_currentInput.braking = false;
    }
  }
}

glm::vec2 AIDriver::findClosestSplinePoint() const {
  if (!m_car || !m_car->getTrack()) return glm::vec2(0.0f);

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);

  auto splinePoints = m_car->getTrack()->getSplinePoints(200);
  float minDist = std::numeric_limits<float>::max();
  glm::vec2 closestPoint;

  for (const auto& point : splinePoints) {
    float dist = glm::length2(carPos - point.position);
    if (dist < minDist) {
      minDist = dist;
      closestPoint = point.position;
    }
  }

  return closestPoint;
}

glm::vec2 AIDriver::calculateLookAheadPoint() const {
  if (!m_car || !m_car->getTrack()) return glm::vec2(0.0f);

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  float speed = debugInfo.currentSpeed;

  // More dramatic speed-based lookahead scaling
  // Base lookahead of 50 units at low speeds, scaling up to 300+ at high speeds
  float adjustedLookAhead = 50.0f + (speed * 0.5f);

  // Cap maximum lookahead to prevent looking too far ahead
  adjustedLookAhead = glm::clamp(adjustedLookAhead, 50.0f, 350.0f);

  auto splinePoints = m_car->getTrack()->getSplinePoints(200);
  bool isClockwise = !m_car->getTrack()->isDefaultDirection();

  // Find closest point index
  size_t closestIdx = 0;
  float minDist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < splinePoints.size(); i++) {
    float dist = glm::length2(carPos - splinePoints[i].position);
    if (dist < minDist) {
      minDist = dist;
      closestIdx = i;
    }
  }

  // Walk along spline in correct direction
  float distanceAlong = 0.0f;
  size_t currentIdx = closestIdx;
  size_t numPoints = splinePoints.size();

  while (distanceAlong < adjustedLookAhead) {
    size_t nextIdx;
    if (isClockwise) {
      nextIdx = (currentIdx == 0) ? numPoints - 1 : currentIdx - 1;
    }
    else {
      nextIdx = (currentIdx + 1) % numPoints;
    }

    float segmentLength = glm::distance(
      splinePoints[currentIdx].position,
      splinePoints[nextIdx].position
    );

    if (distanceAlong + segmentLength > adjustedLookAhead) {
      float t = (adjustedLookAhead - distanceAlong) / segmentLength;
      return glm::mix(
        splinePoints[currentIdx].position,
        splinePoints[nextIdx].position,
        t
      );
    }

    distanceAlong += segmentLength;
    currentIdx = nextIdx;

    // Prevent infinite loop
    if (currentIdx == closestIdx) break;
  }

  return splinePoints[currentIdx].position;
}

float AIDriver::calculateCornerSpeed(const glm::vec2& lookAheadPoint) const {
  if (!m_car) return 0.0f;

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);

  // Calculate angle change to lookahead point
  glm::vec2 toAhead = glm::normalize(lookAheadPoint - carPos);
  glm::vec2 carDir = glm::vec2(cos(debugInfo.angle), sin(debugInfo.angle));
  float turnAngle = acos(glm::dot(toAhead, carDir));

  float angleThreshold = 0.2f;
  float speedMultiplier = 1.0f;

  if (turnAngle > angleThreshold) {
    // More aggressive speed reduction for sharp turns
    float sharpness = (turnAngle - angleThreshold) / (glm::pi<float>() - angleThreshold);
    speedMultiplier = 1.0f - (sharpness * 0.7f);  // Allow down to 30% speed in very sharp turns

    // If we're already drifting, allow slightly higher speeds
    if (m_car->getProperties().driftState > 0.5f) {
      speedMultiplier += 0.2f;
    }

    speedMultiplier = glm::clamp(speedMultiplier, 0.3f, 1.0f);
  }

  return m_car->getProperties().maxSpeed * speedMultiplier;
}

bool AIDriver::shouldBrakeForCorner(float cornerSpeed, float currentSpeed) const {
  // Use a fixed braking buffer instead of a configurable one
  const float BRAKING_BUFFER = 25.0f;
  return currentSpeed > cornerSpeed + BRAKING_BUFFER;
}
