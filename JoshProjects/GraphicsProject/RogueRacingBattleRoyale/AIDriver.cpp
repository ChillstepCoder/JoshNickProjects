// AIDriver.cpp

#include "AIDriver.h"
#include <algorithm>
#include <set>
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

  if (DEBUG_OUTPUT) {
    std::cout << "\n=== AI Update Start ===" << std::endl;
  }

  // Update sensors periodically
  m_sensorTimer += deltaTime;
  if (m_sensorTimer >= SENSOR_UPDATE_INTERVAL) {
    m_sensorTimer = 0.0f;

    if (DEBUG_OUTPUT) {
      if (!m_objectManager) {
        std::cout << "Warning: Object manager is null!" << std::endl;
      }
      else {
        std::cout << "Using object manager at: " << m_objectManager << std::endl;
      }
    }

    updateSensors();
  }

  m_targetPoint = findClosestSplinePoint();
  m_lookAheadPoint = calculateLookAheadPoint();
  updateSteering();
  updateThrottle();
  m_car->update(m_currentInput);

  if (DEBUG_OUTPUT) {
    std::cout << "=== AI Update Complete ===" << std::endl;
  }
}

float AIDriver::calculateObjectThreat(const SensorReading& reading) {
    if (!m_car) return 0.0f;

    // Base distance and angle calculations
    float distanceThreat = 1.0f - (reading.distance / SensorData::SENSOR_RANGE);
    float angleToObject = std::abs(reading.angleToObject);
    
    // Cars - Strong avoidance at all times
    if (reading.car) {
        const float CAR_ANGLE_THRESHOLD = glm::pi<float>() / 3.0f; // 60 degrees
        if (angleToObject < CAR_ANGLE_THRESHOLD) {
            float angleFactor = 1.0f - (angleToObject / CAR_ANGLE_THRESHOLD);
            // Exponential distance threat for cars when close
            float carThreat = (reading.distance < 30.0f) 
                ? std::pow(distanceThreat, 0.5f) 
                : distanceThreat;
            return carThreat * angleFactor * 4.0f; // High base priority for cars
        }
        return 0.0f;
    }

    // Static objects
    if (reading.object) {
        const std::string& objName = reading.object->getDisplayName();
        
        // Solid objects (trees) - Strong avoidance when directly ahead
        if (objName.find("tree") != std::string::npos) {
            const float TREE_ANGLE_THRESHOLD = glm::pi<float>() / 6.0f; // 30 degrees
            if (angleToObject < TREE_ANGLE_THRESHOLD) {
                float angleFactor = 1.0f - (angleToObject / TREE_ANGLE_THRESHOLD);
                return distanceThreat * angleFactor * 5.0f; // Highest priority when in path
            }
            return 0.0f;
        }
        
        // Non-solid objects (cones, potholes) - Path-based avoidance
        glm::vec2 objectPos = reading.object->getPosition();
        glm::vec2 targetToLookahead = m_lookAheadPoint - m_targetPoint;
        glm::vec2 targetToObject = objectPos - m_targetPoint;
        float projection = glm::dot(targetToObject, glm::normalize(targetToLookahead));
        float pathLength = glm::length(targetToLookahead);
        
        // Calculate distance from racing line
        float t = glm::clamp(projection / pathLength, 0.0f, 1.0f);
        glm::vec2 nearestPathPoint = m_targetPoint + t * glm::normalize(targetToLookahead) * pathLength;
        float distanceFromPath = glm::distance(objectPos, nearestPathPoint);

        // Rapidly reduce priority for objects far from racing line
        const float PATH_THRESHOLD = 20.0f;
        float pathThreat = (distanceFromPath > PATH_THRESHOLD) 
            ? std::exp(-(distanceFromPath - PATH_THRESHOLD) / 15.0f) 
            : 1.0f;

        if (objName.find("cone") != std::string::npos) {
            return distanceThreat * pathThreat * 0.7f;
        }
        if (objName.find("pothole") != std::string::npos) {
            return distanceThreat * pathThreat * 0.4f;
        }
    }

    return 0.0f;
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
      objectWeight = 0.8f;  // Cars are similar priority to cones
    }

    // Calculate total threat
    float threat = calculateObjectThreat(reading);

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
    float avoidStrength;

    if (mostThreatening->car) {
      // Wider detection angle (60 degrees each side)
      float maxAngle = glm::pi<float>() / 3.0f; // 60 degrees
      float angleToObject = std::abs(mostThreatening->angleToObject);

      // Base detection range that scales with speed
      float speedBasedRange = 50.0f + (currentSpeed * 0.3f);

      if (angleToObject < maxAngle && mostThreatening->distance < speedBasedRange) {
        // Calculate base avoidance strength
        float distanceThreat = (speedBasedRange - mostThreatening->distance) / speedBasedRange;

        // Strong response to very close cars
        if (mostThreatening->distance < 30.0f) {
          distanceThreat = std::pow(distanceThreat, 0.5f); // Square root to increase close-range response
        }

        // Angle weight with more forgiving falloff for close objects
        float angleWeight;
        if (mostThreatening->distance < 20.0f) {
          // Very close cars get high weight even at wider angles
          angleWeight = 0.8f;
        }
        else {
          // Linear falloff but maintain higher minimum for closer cars
          angleWeight = 1.0f - (angleToObject / maxAngle);
          angleWeight = glm::max(angleWeight, 0.4f);
        }

        // Calculate final avoidance strength
        avoidStrength = distanceThreat * angleWeight * glm::pi<float>() * 0.35f; // Max ~63 degrees

        // Boost avoidance for very close cars
        if (mostThreatening->distance < 15.0f) {
          avoidStrength *= 2.0f;
        }
      }
      else {
        avoidStrength = 0.0f;
      }
    }
    else {
      // Normal strong avoidance for other obstacles
      avoidStrength = highestThreat * glm::pi<float>() * 0.5f; // Up to 90 degrees
    }

    if (avoidStrength > 0.0f) {
      // Determine which direction to turn
      glm::vec2 toTrackCenter = glm::normalize(m_targetPoint - carPos);
      bool turnLeft = glm::cross(glm::vec3(carForward, 0),
        glm::vec3(toTrackCenter, 0)).z > 0;

      // For car avoidance, prefer turning toward track center
      if (mostThreatening->car) {
        turnLeft = glm::cross(glm::vec3(carForward, 0),
          glm::vec3(toTrackCenter, 0)).z > 0;
      }
      // For obstacles, avoid toward the opposite side
      else if (std::abs(mostThreatening->angleToObject) > 0.2f) {
        turnLeft = mostThreatening->angleToObject < 0;
      }

      avoidanceAngle = turnLeft ? avoidStrength : -avoidStrength;
    }
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

  // Calculate how well we're aligned with the track
  float trackAlignmentAngle = acos(glm::dot(carDir, trackDirection));

  // Calculate distance from track
  float distanceFromTrack = glm::length(carPos - m_targetPoint);

  // Base maximum speed on alignment and track distance
  float speedLimit = m_car->getProperties().maxSpeed;
  float alignmentFactor = (glm::pi<float>() - trackAlignmentAngle) / glm::pi<float>();
  alignmentFactor = glm::clamp(alignmentFactor * alignmentFactor, 0.3f, 1.0f);

  // Reduce speed when far from track
  float distanceFactor = 1.0f - glm::clamp(distanceFromTrack / 100.0f, 0.0f, 0.5f);

  // Combined speed factor
  float speedFactor = alignmentFactor * distanceFactor;
  speedLimit *= speedFactor;

  // Additional speed reduction for sharp turns
  float cornerSpeed = calculateCornerSpeed(m_lookAheadPoint);
  speedLimit = std::min(speedLimit, cornerSpeed);

  // Don't slow down just because other cars are nearby
  bool needsBraking = currentSpeed > speedLimit + 25.0f;
  bool needsRecovery = currentSpeed < 50.0f;

  if (needsBraking) {
    m_currentInput.braking = true;
    m_currentInput.accelerating = false;
  }
  else if (needsRecovery) {
    // Always maintain some forward momentum when slow
    float recoveryAngle = std::abs(trackAlignmentAngle);
    m_currentInput.accelerating = recoveryAngle < glm::pi<float>() * 0.7f || currentSpeed < 20.0f;
    m_currentInput.braking = false;
  }
  else {
    // Normal driving - accelerate when below speed limit
    m_currentInput.accelerating = currentSpeed < speedLimit - 25.0f;
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
  if (!m_car || !m_objectManager) return;

  m_sensorData.readings.clear();

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carForward(std::cos(debugInfo.angle), std::sin(debugInfo.angle));

  // Use a hash map to track detected positions with 1-unit grid snapping
  std::unordered_map<int64_t, bool> detectedPositions;
  auto hashPosition = [](const glm::vec2& pos) -> int64_t {
    // Round position to nearest unit to prevent floating point precision issues
    int32_t x = static_cast<int32_t>(std::round(pos.x));
    int32_t y = static_cast<int32_t>(std::round(pos.y));
    return (static_cast<int64_t>(x) << 32) | static_cast<int64_t>(y);
    };

  // First scan for static objects
  auto nearbyObjects = m_objectManager->getNearbyObjects(carPos, SensorData::SENSOR_RANGE);
  for (const auto* obj : nearbyObjects) {
    glm::vec2 objPos = obj->getPosition();
    int64_t posHash = hashPosition(objPos);

    // Skip if we've already detected something at this position
    if (detectedPositions[posHash]) continue;

    float pathDistance, pathAngle;
    if (isObjectInPath(objPos, 10.0f, carPos, carForward, &pathDistance, &pathAngle)) {
      if (pathDistance < SensorData::SENSOR_RANGE) {
        SensorReading reading;
        reading.object = obj;
        reading.distance = pathDistance;
        reading.angleToObject = pathAngle;
        reading.isLeftSide = pathAngle > 0;
        m_sensorData.readings.push_back(reading);
        detectedPositions[posHash] = true;

        if (DEBUG_OUTPUT) {
          std::cout << "Detected " << obj->getDisplayName()
            << " at distance " << pathDistance
            << " angle " << pathAngle << "\n";
        }
      }
    }
  }

  // Then scan for other cars if we have access to them
  const auto& cars = m_objectManager->getCars();
  for (Car* otherCar : cars) {
    if (otherCar == m_car) continue;  // Skip self

    auto otherDebugInfo = otherCar->getDebugInfo();
    glm::vec2 otherPos(otherDebugInfo.position);
    int64_t posHash = hashPosition(otherPos);

    // Skip if we've already detected something at this position
    if (detectedPositions[posHash]) continue;

    float pathDistance, pathAngle;
    if (isObjectInPath(otherPos, 15.0f, carPos, carForward, &pathDistance, &pathAngle)) {
      if (pathDistance < SensorData::SENSOR_RANGE) {
        SensorReading reading;
        reading.car = otherCar;
        reading.distance = pathDistance;
        reading.angleToObject = pathAngle;
        reading.isLeftSide = pathAngle > 0;
        m_sensorData.readings.push_back(reading);
        detectedPositions[posHash] = true;

        if (DEBUG_OUTPUT) {
          std::cout << "Detected car at distance " << pathDistance
            << " angle " << pathAngle << "\n";
        }
      }
    }
  }
}

void AIDriver::scanForObjects() {
  if (!m_objectManager || !m_car) return;

  m_sensorData.readings.clear();

  if (DEBUG_OUTPUT) {
    std::cout << "\n=== Starting new scan ===" << std::endl;
  }

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carForward(std::cos(debugInfo.angle), std::sin(debugInfo.angle));

  // Keep track of already detected object pointers
  std::set<const PlaceableObject*> detectedObjects;

  auto nearbyObjects = m_objectManager->getNearbyObjects(carPos, SensorData::SENSOR_RANGE);
  if (DEBUG_OUTPUT) {
    std::cout << "Found " << nearbyObjects.size() << " nearby objects" << std::endl;
  }

  for (const auto* obj : nearbyObjects) {
    if (detectedObjects.find(obj) != detectedObjects.end()) {
      if (DEBUG_OUTPUT) {
        std::cout << "Skipping already detected object" << std::endl;
      }
      continue;
    }

    float pathDistance, pathAngle;
    if (isObjectInPath(obj->getPosition(), 10.0f, carPos, carForward, &pathDistance, &pathAngle)) {
      if (pathDistance < SensorData::SENSOR_RANGE) {
        if (DEBUG_OUTPUT) {
          std::cout << "Adding new detection: " << obj->getDisplayName()
            << " at distance " << pathDistance
            << " angle " << pathAngle << std::endl;
        }

        SensorReading reading;
        reading.object = obj;
        reading.distance = pathDistance;
        reading.angleToObject = pathAngle;
        reading.isLeftSide = pathAngle > 0;
        m_sensorData.readings.push_back(reading);

        detectedObjects.insert(obj);
      }
    }
  }

  if (DEBUG_OUTPUT) {
    std::cout << "Total readings after scan: " << m_sensorData.readings.size() << std::endl;
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
