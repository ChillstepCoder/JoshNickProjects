// AIDriver.h

#pragma once
#include <glm/glm.hpp>
#include "Car.h"
#include "SplineTrack.h"
#include "InputState.h"

class AIDriver {
public:
  struct Config {
    float lookAheadDistance = 100.0f;      // How far ahead to look for turns
    float centeringForce = 1.0f;           // How strongly to center on spline (0-1)
    float turnAnticipation = 1.0f;         // How much to consider lookahead angle (0-1)
    float reactionTime = 0.1f;             // Delay in steering response
  };

  AIDriver(Car* car);

  void update(float deltaTime);

  // Getters/Setters
  void setConfig(const Config& config) { m_config = config; }
  const Config& getConfig() const { return m_config; }

  // Debug info
  glm::vec2 getTargetPoint() const { return m_targetPoint; }
  glm::vec2 getLookAheadPoint() const { return m_lookAheadPoint; }
  float getDesiredAngle() const { return m_desiredAngle; }

private:
  Car* m_car;
  Config m_config;

  // Cached state
  glm::vec2 m_targetPoint;
  glm::vec2 m_lookAheadPoint;
  float m_desiredAngle;
  float m_lastInputTime;
  InputState m_currentInput;

  // Helper methods
  void updateSteering();
  void updateThrottle();
  glm::vec2 findClosestSplinePoint() const;
  glm::vec2 calculateLookAheadPoint() const;
  float calculateTargetAngle() const;
  float calculateCornerSpeed(const glm::vec2& lookAheadPoint) const;
  bool shouldBrakeForCorner(float cornerSpeed, float currentSpeed) const;
  glm::vec2 calculateRacingLineOffset(const glm::vec2& splinePoint, const glm::vec2& lookAheadPoint) const;
};
