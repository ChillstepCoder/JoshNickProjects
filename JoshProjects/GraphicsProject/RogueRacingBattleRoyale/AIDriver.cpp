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

  // Update sensors periodically
  m_sensorTimer += deltaTime;
  if (m_sensorTimer >= SENSOR_UPDATE_INTERVAL) {
    m_sensorTimer = 0.0f;

    // Find which car number we are
    if (DEBUG_OUTPUT) {
      std::cout << "\n=== Starting AI update for car at position ("
        << m_car->getDebugInfo().position.x << ", "
        << m_car->getDebugInfo().position.y << ") ===\n";
    }
    // Clear sensor data and scan for objects
    m_sensorData.readings.clear();

    if (!m_objectManager) {
      if (DEBUG_OUTPUT) {
        std::cout << "Object manager is null for this car!\n";
        std::cout << "Car angle: " << m_car->getDebugInfo().angle << "\n";
        std::cout << "Car speed: " << m_car->getDebugInfo().currentSpeed << "\n";
      }
    }
    else {
      if (DEBUG_OUTPUT) {
        std::cout << "Object manager address: " << m_objectManager << "\n";
      }
      scanForObjects();
    }
  }

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


void AIDriver::updateSensors() {
  if (!m_car) return;

  m_sensorData.readings.clear();

  // Get car's current state
  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carForward(std::cos(debugInfo.angle), std::sin(debugInfo.angle));

  // Scan for objects
  if (m_objectManager) {
    scanForObjects();
  }
}

void AIDriver::scanForObjects() {
  if (!m_objectManager || !m_car) return;

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carForward(std::cos(debugInfo.angle), std::sin(debugInfo.angle));

  // Only get nearby objects
  auto nearbyObjects = m_objectManager->getNearbyObjects(carPos, SensorData::SENSOR_RANGE);
  if (DEBUG_OUTPUT) {
    std::cout << "\nScanning for objects from car at (" << carPos.x << ", " << carPos.y
      << ") facing " << debugInfo.angle << " radians\n";
    std::cout << "Found " << nearbyObjects.size() << " nearby objects\n";
  }

  for (const auto* obj : nearbyObjects) {
    glm::vec2 objPos = obj->getPosition();
    float straightDistance = glm::distance(carPos, objPos);
    float dx = objPos.x - carPos.x;
    float dy = objPos.y - carPos.y;
    float angleToObject = std::atan2(dy, dx);

    if (DEBUG_OUTPUT) {
      std::cout << "  Checking " << obj->getDisplayName()
        << " at (" << objPos.x << ", " << objPos.y
        << ") distance: " << straightDistance
        << " angle: " << angleToObject << "\n";
    }

    float pathDistance, pathAngle;
    bool inPath = isObjectInPath(objPos, 10.0f, carPos, carForward, &pathDistance, &pathAngle);

    if (inPath) {
      if (DEBUG_OUTPUT) {
        std::cout << "    Object in path! Distance: " << pathDistance
          << ", Angle: " << pathAngle << "\n";
      }
      if (pathDistance < SensorData::SENSOR_RANGE) {
        SensorReading reading;
        reading.object = obj;
        reading.distance = pathDistance;
        reading.angleToObject = pathAngle;
        reading.isLeftSide = pathAngle > 0;
        m_sensorData.readings.push_back(reading);
        if (DEBUG_OUTPUT) {
          std::cout << "    Added to sensor readings\n";
        }
      }
      else {
        if (DEBUG_OUTPUT) {
          std::cout << "    Too far away (beyond sensor range)\n";
        }
      }
    }
  }
}

void AIDriver::scanForCars(const std::vector<Car*>& cars) {
  if (!m_car) return;

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carForward(std::cos(debugInfo.angle), std::sin(debugInfo.angle));

  for (Car* otherCar : cars) {
    if (otherCar == m_car) continue;  // Skip self

    auto otherDebugInfo = otherCar->getDebugInfo();
    float distance, angle;

    if (isObjectInPath(glm::vec2(otherDebugInfo.position), 15.0f,
      carPos, carForward, &distance, &angle)) {
      if (distance < SensorData::SENSOR_RANGE) {
        SensorReading reading;
        reading.car = otherCar;
        reading.distance = distance;
        reading.angleToObject = angle;
        reading.isLeftSide = angle > 0;
        m_sensorData.readings.push_back(reading);
      }
    }
  }
}

bool AIDriver::isObjectInPath(const glm::vec2& objectPos, float objectRadius,
  const glm::vec2& carPos, const glm::vec2& carForward,
  float* outDistance, float* outAngle) {
  glm::vec2 toObject = objectPos - carPos;
  float distance = glm::length(toObject);

  // If object is behind us, ignore it
  float forwardDot = glm::dot(toObject, carForward);
  if (forwardDot < 0) {
    if (DEBUG_OUTPUT) {
      std::cout << "Object is behind car\n";
    }
    return false;
  }

  // Calculate lateral distance from car's forward path
  glm::vec2 normalizedToObject = toObject / distance;
  float angle = std::atan2(normalizedToObject.y, normalizedToObject.x) -
    std::atan2(carForward.y, carForward.x);

  // Normalize angle to [-π, π]
  while (angle > glm::pi<float>()) angle -= 2.0f * glm::pi<float>();
  while (angle < -glm::pi<float>()) angle += 2.0f * glm::pi<float>();

  float lateralDist = std::abs(distance * std::sin(angle));

  if (outDistance) *outDistance = distance;
  if (outAngle) *outAngle = angle;

  // Add debug output
  if (DEBUG_OUTPUT) {
    std::cout << "Object check - Lateral distance: " << lateralDist
      << ", Sensor width: " << SensorData::SENSOR_WIDTH
      << ", Object radius: " << objectRadius << "\n";
  }

  // Check if object is within our sensor width
  return lateralDist < (SensorData::SENSOR_WIDTH + objectRadius);
}
