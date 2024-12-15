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

  if (DEBUG_OUTPUT && m_stuckState.isStuck) {
    auto debugInfo = m_car->getDebugInfo();
    std::cout << "Stuck Recovery Active - Speed: " << debugInfo.currentSpeed
      << " Distance: " << glm::distance(glm::vec2(debugInfo.position),
        m_stuckState.stuckPosition) << std::endl;
  }

  // Update stuck detection
  updateStuckState(deltaTime);

  // Update sensors periodically
  m_sensorTimer += deltaTime;
  if (m_sensorTimer >= SENSOR_UPDATE_INTERVAL) {
    m_sensorTimer = 0.0f;
    updateSensors();
  }

  m_targetPoint = findClosestSplinePoint();
  m_lookAheadPoint = calculateLookAheadPoint();

  // Normal driving behavior if not stuck
  if (!m_stuckState.isStuck) {
    updateSteering();
    updateThrottle();
  }
  else {
    // Recovery behavior when stuck
    applyStuckRecovery();
  }

  m_car->update(m_currentInput);
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

  // Calculate track direction at current position
  glm::vec2 trackDir = getTrackDirectionAtPosition(carPos);
  float trackAlignment = glm::dot(carForward, trackDir);

  // Calculate vector to target spline point and its perpendicular
  glm::vec2 toTarget = m_targetPoint - carPos;
  float distanceToSpline = glm::length(toTarget);
  toTarget = glm::normalize(toTarget);

  // Calculate perpendicular vector to track direction (for offset calculation)
  glm::vec2 trackNormal = glm::vec2(-trackDir.y, trackDir.x);

  // Calculate which side of the track we're on and current offset
  float currentOffset = glm::dot(toTarget, trackNormal);
  float desiredOffset = 0.0f; // Could be adjusted for racing line optimization

  // Calculate future track direction for drift detection
  glm::vec2 futureTrackDir = getTrackDirectionAtPosition(m_lookAheadPoint);
  float turnAngle = std::acos(glm::clamp(glm::dot(trackDir, futureTrackDir), -1.0f, 1.0f));

  // Get car properties for drift handling
  auto& props = m_car->getProperties();
  float driftState = props.driftState;

  // Calculate desired direction
  glm::vec2 desiredDir;

  if (driftState > 0.5f) {
    // During drift, use more aggressive steering based on turn direction
    float turnDir = glm::cross(glm::vec3(trackDir, 0), glm::vec3(futureTrackDir, 0)).z;
    float driftAngle = std::abs(turnAngle);

    if (driftAngle > 0.1f) {
      // Calculate a more aggressive desired direction during drift
      glm::vec2 turnBias = glm::vec2(-trackDir.y, trackDir.x) * glm::sign(turnDir);
      float aggressiveness = glm::min(1.0f, driftAngle / (glm::pi<float>() * 0.5f)) * 1.5f;
      desiredDir = glm::normalize(trackDir + turnBias * aggressiveness);
    }
    else {
      desiredDir = trackDir; // Straighten out if turn isn't sharp
    }
  }
  else {
    // Normal steering behavior when not drifting
    if (trackAlignment > 0.9f) {
      if (std::abs(distanceToSpline - desiredOffset) < 5.0f) {
        desiredDir = trackDir;
      }
      else {
        float correctionStrength = glm::min(1.0f, std::abs(distanceToSpline - desiredOffset) / 20.0f);
        desiredDir = glm::normalize(trackDir + toTarget * correctionStrength * 0.5f);
      }
    }
    else {
      float alignmentNeeded = 1.0f - trackAlignment;
      desiredDir = glm::normalize(trackDir + toTarget * alignmentNeeded);
    }
  }

  // Calculate final desired angle
  m_desiredAngle = atan2(desiredDir.y, desiredDir.x);

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
  if (needsAvoidance) {
    // Scale avoidance by speed - less aggressive at high speeds
    float speedFactor = glm::clamp(currentSpeed / 500.0f, 0.0f, 1.0f);
    float avoidanceWeight = 1.0f - (speedFactor * 0.5f);
    // Add avoidance to our already calculated desired angle
    m_desiredAngle += avoidanceAngle * avoidanceWeight;
  }

  // Calculate final steering angle
  float angleDiff = m_desiredAngle - carAngle;
  while (angleDiff > glm::pi<float>()) angleDiff -= 2.0f * glm::pi<float>();
  while (angleDiff < -glm::pi<float>()) angleDiff += 2.0f * glm::pi<float>();

  // Apply steering with consideration for drift state
  float steeringThreshold;
  if (driftState > 0.5f) {
    steeringThreshold = 0.01f; // Smaller threshold during drift for more responsive steering
  }
  else {
    steeringThreshold = trackAlignment > 0.95f ? 0.03f : 0.02f;
  }

  if (std::abs(angleDiff) > steeringThreshold) {
    m_currentInput.turningLeft = angleDiff > 0;
    m_currentInput.turningRight = angleDiff < 0;
  }
  else {
    m_currentInput.turningLeft = false;
    m_currentInput.turningRight = false;
  }
}

void AIDriver::updateThrottle() {
  auto debugInfo = m_car->getDebugInfo();
  float currentSpeed = debugInfo.currentSpeed;
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carDir = glm::vec2(cos(debugInfo.angle), sin(debugInfo.angle));

  // Calculate track direction and upcoming turn severity
  glm::vec2 currentTrackDir = getTrackDirectionAtPosition(carPos);
  glm::vec2 futureTrackDir = getTrackDirectionAtPosition(m_lookAheadPoint);
  float turnAngle = std::acos(glm::clamp(glm::dot(currentTrackDir, futureTrackDir), -1.0f, 1.0f));

  // Calculate how well we're aligned with the track
  float trackAlignmentAngle = acos(glm::dot(carDir, currentTrackDir));

  // Base maximum speed on alignment and turn severity
  float speedLimit = m_car->getProperties().maxSpeed;
  float turnSeverity = turnAngle / (glm::pi<float>() * 0.5f); // 0 to 1 for up to 90-degree turns

  // More aggressive speed reduction in sharp turns
  if (turnSeverity > 0.2f) {
    float cornerSpeedFactor = 1.0f - (turnSeverity * 0.8f);
    speedLimit *= cornerSpeedFactor;
  }

  // Determine if we should initiate drift
  bool shouldDrift = turnSeverity > 0.3f && currentSpeed > 200.0f;
  bool currentlyDrifting = m_car->getProperties().driftState > 0.5f;

  // Initiate drift with brake tap if needed
  if (shouldDrift && !currentlyDrifting) {
    m_currentInput.braking = true;
    m_currentInput.accelerating = false;
  }
  else {
    // Normal speed control
    bool needsBraking = currentSpeed > speedLimit + 50.0f;

    if (needsBraking) {
      m_currentInput.braking = true;
      m_currentInput.accelerating = false;
    }
    else {
      m_currentInput.braking = false;
      m_currentInput.accelerating = currentSpeed < speedLimit - 25.0f;
    }
  }

  // Keep some acceleration during drift to maintain speed
  if (currentlyDrifting && currentSpeed < speedLimit * 0.7f) {
    m_currentInput.accelerating = true;
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

  // Calculate immediate turn angle
  glm::vec2 toAhead = glm::normalize(lookAheadPoint - carPos);
  glm::vec2 carDir = glm::vec2(cos(debugInfo.angle), sin(debugInfo.angle));
  float turnAngle = acos(glm::dot(toAhead, carDir));

  // Get track direction at current and future positions
  glm::vec2 currentTrackDir = getTrackDirectionAtPosition(carPos);
  glm::vec2 futureTrackDir = getTrackDirectionAtPosition(lookAheadPoint);

  // Calculate track curvature
  float trackAngleChange = acos(glm::dot(currentTrackDir, futureTrackDir));

  float angleThreshold = 0.2f;
  float speedMultiplier = 1.0f;

  if (turnAngle > angleThreshold || trackAngleChange > angleThreshold) {
    // Use the larger of the two angles for speed reduction
    float effectiveAngle = glm::max(turnAngle, trackAngleChange);
    float sharpness = (effectiveAngle - angleThreshold) / (glm::pi<float>() - angleThreshold);

    // More gradual speed reduction curve
    speedMultiplier = 1.0f - (sharpness * 0.6f);  // Changed from 0.7f

    // Less speed reduction when drifting
    if (m_car->getProperties().driftState > 0.5f) {
      speedMultiplier = glm::min(speedMultiplier + 0.3f, 1.0f);
    }

    speedMultiplier = glm::clamp(speedMultiplier, 0.4f, 1.0f);
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

void AIDriver::updateStuckState(float deltaTime) {
  if (!m_car) return;

  auto debugInfo = m_car->getDebugInfo();
  float currentSpeed = debugInfo.currentSpeed;
  glm::vec2 currentPos(debugInfo.position);

  // Update position check timer
  m_stuckState.lastPositionTimer += deltaTime;
  if (m_stuckState.lastPositionTimer >= StuckState::POSITION_CHECK_INTERVAL) {
    float distanceMoved = glm::distance(currentPos, m_stuckState.lastPosition);
    float effectiveSpeed = distanceMoved / StuckState::POSITION_CHECK_INTERVAL;

    // Reset timer and update last position
    m_stuckState.lastPositionTimer = 0.0f;
    m_stuckState.lastPosition = currentPos;

    // If speed is below threshold, increment appropriate stuck timer
    if (effectiveSpeed < m_config.stuckSpeedThreshold) {
      if (m_stuckState.isStuck) {
        // In reverse mode, track getting stuck while reversing
        m_stuckState.reverseStuckTimer += StuckState::POSITION_CHECK_INTERVAL;
        if (m_stuckState.reverseStuckTimer >= m_config.stuckTimeThreshold) {
          // Got stuck while reversing - abort recovery
          if (DEBUG_OUTPUT) {
            std::cout << "Got stuck while reversing - aborting recovery" << std::endl;
          }
          m_stuckState.isStuck = false;
          m_stuckState.stuckTimer = 0.0f;
          m_stuckState.reverseStuckTimer = 0.0f;
          m_stuckState.recoveryTimer = 0.0f;
          return;
        }
      }
      else {
        // Normal forward stuck detection
        m_stuckState.stuckTimer += StuckState::POSITION_CHECK_INTERVAL;
      }
    }
    else {
      // Moving well - reset timers
      m_stuckState.stuckTimer = 0.0f;
      m_stuckState.reverseStuckTimer = 0.0f;
    }
  }

  // Check if we should enter stuck state
  if (!m_stuckState.isStuck && m_stuckState.stuckTimer >= m_config.stuckTimeThreshold) {
    m_stuckState.isStuck = true;
    m_stuckState.stuckPosition = currentPos;  // Store position where we got stuck
    m_stuckState.reverseStuckTimer = 0.0f;   // Reset reverse stuck timer
    m_stuckState.recoveryTimer = 0.0f;       // Reset recovery timer
  }

  // Update recovery timer and check exit conditions
  if (m_stuckState.isStuck) {
    m_stuckState.recoveryTimer += deltaTime;
    float distanceFromStuckPoint = glm::distance(currentPos, m_stuckState.stuckPosition);

    // Exit stuck state if either condition is met
    if (distanceFromStuckPoint >= m_config.recoveryDistance ||
      m_stuckState.recoveryTimer >= m_config.recoveryMaxTime) {

      if (DEBUG_OUTPUT) {
        if (distanceFromStuckPoint >= m_config.recoveryDistance) {
          std::cout << "Recovery complete - distance threshold reached" << std::endl;
        }
        else {
          std::cout << "Recovery complete - time threshold reached" << std::endl;
        }
      }

      // Exit stuck state
      m_stuckState.isStuck = false;
      m_stuckState.stuckTimer = 0.0f;
      m_stuckState.reverseStuckTimer = 0.0f;
      m_stuckState.recoveryTimer = 0.0f;
    }
  }
}

glm::vec2 AIDriver::getTrackDirectionAtPosition(const glm::vec2& position) const {
  if (!m_car || !m_car->getTrack()) return glm::vec2(1.0f, 0.0f);

  auto splinePoints = m_car->getTrack()->getSplinePoints(200);
  size_t closestIdx = 0;
  float minDist = FLT_MAX;

  // Find closest spline point
  for (size_t i = 0; i < splinePoints.size(); i++) {
    float dist = glm::length2(position - splinePoints[i].position);
    if (dist < minDist) {
      minDist = dist;
      closestIdx = i;
    }
  }

  // Get next point for direction (considering track direction)
  bool isClockwise = !m_car->getTrack()->isDefaultDirection();
  size_t nextIdx = isClockwise ?
    (closestIdx == 0 ? splinePoints.size() - 1 : closestIdx - 1) :
    (closestIdx + 1) % splinePoints.size();

  glm::vec2 direction = glm::normalize(splinePoints[nextIdx].position - splinePoints[closestIdx].position);
  return isClockwise ? -direction : direction;
}

void AIDriver::applyStuckRecovery() {
  if (!m_car) return;

  auto debugInfo = m_car->getDebugInfo();
  glm::vec2 carPos(debugInfo.position);
  glm::vec2 carDir(std::cos(debugInfo.angle), std::sin(debugInfo.angle));

  // Get closest spline point and track direction
  glm::vec2 closestSplinePoint = findClosestSplinePoint();
  glm::vec2 trackDir = getTrackDirectionAtPosition(carPos);

  // Calculate vector from car to spline center
  glm::vec2 toCenter = closestSplinePoint - carPos;
  float distanceToSpline = glm::length(toCenter);
  toCenter = glm::normalize(toCenter);

  // Calculate ideal reverse direction (opposite of track direction)
  glm::vec2 reverseDir = -trackDir;

  // Mix reverse direction with centering force
  float centeringStrength = glm::clamp(distanceToSpline / 50.0f, 0.0f, 1.0f);
  centeringStrength *= m_config.centeringForce; // Use existing centering force config

  // Blend between pure reverse direction and centering direction
  glm::vec2 targetDir = glm::normalize(
    reverseDir + toCenter * centeringStrength * 1.5f  // Increased centering force while reversing
  );

  // Calculate the angle we want to achieve
  float targetAngle = std::atan2(targetDir.y, targetDir.x);
  float currentAngle = debugInfo.angle;

  // Normalize angle difference to [-π, π]
  float angleDiff = targetAngle - currentAngle;
  while (angleDiff > glm::pi<float>()) angleDiff -= 2.0f * glm::pi<float>();
  while (angleDiff < -glm::pi<float>()) angleDiff += 2.0f * glm::pi<float>();

  // Dynamic steering threshold based on distance to spline
  float steeringThreshold = 0.1f;  // Base threshold (about 5.7 degrees)
  if (distanceToSpline > 30.0f) {
    steeringThreshold *= 0.5f;  // More precise steering when far from spline
  }

  // Apply steering based on angle difference
  m_currentInput.turningLeft = angleDiff > steeringThreshold;
  m_currentInput.turningRight = angleDiff < -steeringThreshold;

  // Adjust reverse speed based on alignment and distance
  m_currentInput.accelerating = false;
  m_currentInput.braking = true;

  // Debug output if needed
  if (DEBUG_OUTPUT) {
    std::cout << "Recovery - Angle diff: " << glm::degrees(angleDiff)
      << " Distance to spline: " << distanceToSpline
      << " Centering strength: " << centeringStrength
      << " Distance from stuck point: "
      << glm::distance(carPos, m_stuckState.stuckPosition) << std::endl;
  }
}
