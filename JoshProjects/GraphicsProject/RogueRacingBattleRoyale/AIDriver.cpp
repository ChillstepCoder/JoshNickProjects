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

  // Always update target points considering track direction
  m_targetPoint = findClosestSplinePoint();
  m_lookAheadPoint = calculateLookAheadPoint();

  // Update steering every frame - removed reaction time delay
  updateSteering();

  // Always update throttle every frame
  updateThrottle();

  // Apply inputs to car
  m_car->update(m_currentInput);
}


void AIDriver::updateSteering() {
  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  float carAngle = debugInfo.angle;
  float currentSpeed = debugInfo.currentSpeed;
  glm::vec2 carForward(std::cos(carAngle), std::sin(carAngle));

  // Calculate base path following behavior
  glm::vec2 toTarget = m_targetPoint - carPos;
  glm::vec2 toLookAhead = m_lookAheadPoint - carPos;
  float targetAngle = atan2(toTarget.y, toTarget.x);
  float lookAheadAngle = atan2(toLookAhead.y, toLookAhead.x);

  // Calculate base desired angle
  float baseDesiredAngle = targetAngle * (1.0f - m_config.turnAnticipation) +
    lookAheadAngle * m_config.turnAnticipation;

  // Handle obstacle avoidance
  float avoidanceAngle = 0.0f;
  float closestDist = SensorData::SENSOR_RANGE;
  bool needsAvoidance = false;

  // Find most threatening obstacle
  const SensorReading* mostThreatening = nullptr;
  float highestThreat = 0.0f;

  for (const auto& reading : m_sensorData.readings) {
    if (reading.distance > SensorData::SENSOR_RANGE) continue;

    // Calculate base threat based on distance
    float distanceFactor = 1.0f - (reading.distance / SensorData::SENSOR_RANGE);

    // Weight by angle - objects directly ahead are more threatening
    float angleWeight = std::cos(reading.angleToObject);
    if (angleWeight < 0) continue; // Ignore objects behind our forward arc

    // Calculate threat level based on object type
    float objectWeight = 1.0f;
    if (reading.object) {
      const std::string& objName = reading.object->getDisplayName();
      if (objName.find("tree") != std::string::npos) {
        objectWeight = 5.0f;  // Trees are highest priority
      }
      else if (objName.find("cone") != std::string::npos) {
        objectWeight = 0.9f;  // Cones are lower priority
      }
      else if (objName.find("pothole") != std::string::npos) {
        objectWeight = 0.5f;  // Potholes are lowest priority
      }
    }
    else if (reading.car) {
      objectWeight = 3.0f;  // Other cars are high priority
    }

    // Calculate total threat
    float threat = distanceFactor * angleWeight * objectWeight;

    // Update most threatening obstacle
    if (threat > highestThreat) {
      highestThreat = threat;
      mostThreatening = &reading;
      needsAvoidance = true;
      closestDist = reading.distance;
    }
  }

  // Calculate avoidance steering if needed
  if (needsAvoidance && mostThreatening) {
    // Base avoidance angle on the object's angle
    float avoidStrength = highestThreat * glm::pi<float>() * 0.5f; // Up to 90 degrees

    // Determine which direction to turn
    glm::vec2 toTrackCenter = glm::normalize(m_targetPoint - carPos);
    bool turnLeft = glm::cross(glm::vec3(carForward, 0),
      glm::vec3(toTrackCenter, 0)).z > 0;

    // If object is significantly to one side, avoid toward the other side
    if (std::abs(mostThreatening->angleToObject) > 0.2f) {
      turnLeft = mostThreatening->angleToObject < 0;
    }

    avoidanceAngle = turnLeft ? avoidStrength : -avoidStrength;
  }

  // Combine path following with avoidance
  m_desiredAngle = baseDesiredAngle;
  if (needsAvoidance) {
    // Scale avoidance by speed - less aggressive at high speeds
    float speedFactor = glm::clamp(currentSpeed / 500.0f, 0.0f, 1.0f);
    float avoidanceWeight = 1.0f - (speedFactor * 0.5f);
    m_desiredAngle += avoidanceAngle * avoidanceWeight;
  }

  // Calculate final steering angle
  float angleDiff = m_desiredAngle - carAngle;
  while (angleDiff > glm::pi<float>()) angleDiff -= 2.0f * glm::pi<float>();
  while (angleDiff < -glm::pi<float>()) angleDiff += 2.0f * glm::pi<float>();

  // Apply steering with minimal dead zone
  float steeringThreshold = 0.02f;
  m_currentInput.turningLeft = angleDiff > steeringThreshold;
  m_currentInput.turningRight = angleDiff < -steeringThreshold;
}

void AIDriver::updateThrottle() {
  auto debugInfo = m_car->getDebugInfo();
  float currentSpeed = debugInfo.currentSpeed;
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carDir = glm::vec2(cos(debugInfo.angle), sin(debugInfo.angle));

  // Calculate alignment with track
  glm::vec2 toAhead = glm::normalize(m_lookAheadPoint - carPos);
  float alignmentAngle = acos(glm::dot(toAhead, carDir));

  // Calculate track direction at current position
  auto splinePoints = m_car->getTrack()->getSplinePoints(200);
  glm::vec2 trackDirection = glm::vec2(0.0f);
  float minDist = FLT_MAX;
  for (size_t i = 0; i < splinePoints.size(); i++) {
    float dist = glm::length2(carPos - splinePoints[i].position);
    if (dist < minDist) {
      minDist = dist;
      size_t nextIdx = (i + 1) % splinePoints.size();
      trackDirection = glm::normalize(
        splinePoints[nextIdx].position - splinePoints[i].position
      );
    }
  }

  // Calculate how well we're aligned with the track's direction
  float trackAlignmentAngle = acos(glm::dot(carDir, trackDirection));

  // Calculate distance from track
  float distanceFromTrack = glm::length(carPos - m_targetPoint);

  // Drift handling
  const float DRIFT_SPEED_THRESHOLD = 200.0f;
  const float DRIFT_ANGLE_THRESHOLD = 0.4f; // About 23 degrees
  bool shouldDrift = currentSpeed > DRIFT_SPEED_THRESHOLD && alignmentAngle > DRIFT_ANGLE_THRESHOLD;

  // Base maximum speed on alignment and track distance
  float speedLimit = m_car->getProperties().maxSpeed;

  if (!shouldDrift) {
    // Reduce speed when not aligned with track (unless drifting)
    float alignmentFactor = (glm::pi<float>() - trackAlignmentAngle) / glm::pi<float>();
    alignmentFactor = glm::clamp(alignmentFactor * alignmentFactor, 0.2f, 1.0f);

    // Further reduce speed when far from track
    float distanceFactor = 1.0f - glm::clamp(distanceFromTrack / 200.0f, 0.0f, 0.7f);

    // Combined speed factor
    float speedFactor = alignmentFactor * distanceFactor;
    speedLimit *= speedFactor;

    // Additional speed reduction for sharp turns
    float cornerSpeed = calculateCornerSpeed(m_lookAheadPoint);
    speedLimit = std::min(speedLimit, cornerSpeed);
  }

  if (shouldDrift) {
    // Initiate drift by braking briefly and turning
    m_currentInput.braking = true;
    m_currentInput.accelerating = false;
  }
  else if (currentSpeed > speedLimit + 25.0f) {
    // Need to slow down
    m_currentInput.braking = true;
    m_currentInput.accelerating = false;
  }
  // Recovery behavior when speed is very low
  else if (currentSpeed < 50.0f) {
    // Always maintain some forward momentum when stuck
    float recoveryAngle = std::abs(trackAlignmentAngle);

    // If we're somewhat aligned with the track, accelerate more
    if (recoveryAngle < glm::pi<float>() * 0.6f) {
      m_currentInput.accelerating = true;
      m_currentInput.braking = false;
    }
    else {
      // Even if poorly aligned, maintain minimum speed
      m_currentInput.accelerating = currentSpeed < 20.0f;
      m_currentInput.braking = false;
    }
  }
  else if (currentSpeed < speedLimit - 25.0f && alignmentAngle < glm::pi<float>() * 0.4f) {
    // Normal acceleration when well-aligned
    m_currentInput.accelerating = true;
    m_currentInput.braking = false;
  }
  else {
    // Coast if at appropriate speed
    m_currentInput.accelerating = false;
    m_currentInput.braking = false;
  }

  // Emergency brake if severely misaligned at high speed
  if (trackAlignmentAngle > glm::pi<float>() * 0.7f && currentSpeed > DRIFT_SPEED_THRESHOLD) {
    m_currentInput.braking = true;
    m_currentInput.accelerating = false;
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
