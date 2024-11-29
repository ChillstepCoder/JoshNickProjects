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

  // Only update inputs after reaction time has passed
  m_lastInputTime += deltaTime;
  if (m_lastInputTime >= m_config.reactionTime) {
    m_lastInputTime = 0.0f;

    // Update target points considering track direction
    m_targetPoint = findClosestSplinePoint();
    m_lookAheadPoint = calculateLookAheadPoint();

    // Update steering and throttle
    updateSteering();
    updateThrottle();

    // Apply inputs to car
    m_car->update(m_currentInput);
  }
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

  // Calculate target speed based on corner
  float cornerSpeed = calculateCornerSpeed(m_lookAheadPoint);
  float maxTargetSpeed = m_car->getProperties().maxSpeed * m_config.maxSpeed;
  float targetSpeed = std::min(maxTargetSpeed, cornerSpeed);

  // Brake if we're going too fast for the upcoming corner
  if (shouldBrakeForCorner(cornerSpeed, currentSpeed)) {
    m_currentInput.braking = true;
    m_currentInput.accelerating = false;
  }
  else {
    // Accelerate if we're under target speed
    m_currentInput.accelerating = currentSpeed < targetSpeed;
    m_currentInput.braking = false;
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

  // Adjust lookahead distance based on speed
  float adjustedLookAhead = m_config.lookAheadDistance * (1.0f + speed / 1000.0f);

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

  // Reduce target speed based on turn sharpness
  float angleThreshold = 0.1f; // Minimum angle before speed reduction
  float speedMultiplier = 1.0f;

  if (turnAngle > angleThreshold) {
    speedMultiplier = 1.0f - (turnAngle - angleThreshold) * m_config.brakingSkill;
    speedMultiplier = glm::clamp(speedMultiplier, 0.3f, 1.0f);
  }

  return m_car->getProperties().maxSpeed * speedMultiplier;
}

bool AIDriver::shouldBrakeForCorner(float cornerSpeed, float currentSpeed) const {
  // Factor in the AI's braking skill
  float brakingBuffer = 50.0f * (1.0f - m_config.brakingSkill);
  return currentSpeed > cornerSpeed + brakingBuffer;
}
