// AIDriver.h

#pragma once
#include <glm/glm.hpp>
#include <limits>
#include "Car.h"
#include "SplineTrack.h"
#include "InputState.h"
#include "ObjectManager.h"
#include "PlaceableObject.h"

class AIDriver {
public:
  struct Config {
    float lookAheadDistance = 200.0f;      // How far ahead to look for turns
    float centeringForce = 1.0f;           // How strongly to center on spline (0-1)
    float turnAnticipation = 1.0f;         // How much to consider lookahead angle (0-1)
    float reactionTime = 0.1f;             // Delay in steering response
  };

  struct SensorReading {
    const PlaceableObject* object = nullptr;
    Car* car = nullptr;
    float distance = FLT_MAX;
    float angleToObject = 0.0f;  // Angle relative to car's forward direction
    bool isLeftSide = false;     // Is the object on the left side of the car's path?
  };

  struct SensorData {
    std::vector<SensorReading> readings;
    static constexpr float SENSOR_RANGE = 100.0f;        // How far to look ahead
    static constexpr float SENSOR_WIDTH = 50.0f;         // How wide to scan
    static constexpr float MIN_DISTANCE = 20.0f;         // Minimum distance to maintain
    static constexpr float SIDE_SENSOR_ANGLE = 0.5f;     // ~30 degrees for side sensors
  };

  AIDriver(Car* car);

  void update(float deltaTime);

  // Setters
  void setConfig(const Config& config) { m_config = config; }
  void setObjectManager(ObjectManager* manager) { m_objectManager = manager; }

  // Getters
  const Config& getConfig() const { return m_config; }
  const SensorData& getSensorData() const { return m_sensorData; }

  // Debug info
  glm::vec2 getTargetPoint() const { return m_targetPoint; }
  glm::vec2 getLookAheadPoint() const { return m_lookAheadPoint; }
  float getDesiredAngle() const { return m_desiredAngle; }

private:
  static constexpr bool DEBUG_OUTPUT = false;
  static constexpr float SENSOR_UPDATE_INTERVAL = 0.1f;
  float m_sensorTimer = 0.0f;


  Car* m_car;
  Config m_config;
  ObjectManager* m_objectManager;

  // Cached state
  glm::vec2 m_targetPoint;
  glm::vec2 m_lookAheadPoint;
  float m_desiredAngle;
  float m_lastInputTime;
  InputState m_currentInput;
  SensorData m_sensorData;

  // Helper methods
  float calculateObjectThreat(const SensorReading& reading);
  void updateSensors();
  void updateSteering();
  void updateThrottle();
  void scanForObjects();
  void scanForCars(const std::vector<Car*>& cars);
  bool isObjectInPath(const glm::vec2& objectPos, float objectRadius,
    const glm::vec2& carPos, const glm::vec2& carForward,
    float* outDistance = nullptr, float* outAngle = nullptr);
  glm::vec2 findClosestSplinePoint() const;
  glm::vec2 calculateLookAheadPoint() const;
  float calculateCornerSpeed(const glm::vec2& lookAheadPoint) const;
  bool shouldBrakeForCorner(float cornerSpeed, float currentSpeed) const;

};
