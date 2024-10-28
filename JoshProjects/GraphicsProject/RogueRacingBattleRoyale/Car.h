// Car.h
#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include "InputState.h"

class Car {
public:
  struct CarProperties {
    float acceleration = 5000.0f;    // Forward acceleration force
    float maxSpeed = 200.0f;         // Maximum forward speed
    float turnSpeed = 4.0f;          // Base turning speed
    float lateralDamping = 0.95f;    // How quickly lateral velocity is reduced
    float dragFactor = 0.99f;        // Air resistance
    float turnResetRate = 0.9f;      // How quickly turning resets
    float maxAngularVelocity = 2.0f; // Maximum rotation speed
    float brakingForce = 0.5f;       // Braking force multiplier
    float minSpeedForTurn = 1.0f;    // Minimum speed required to turn
  };

  struct DebugInfo {
    glm::vec2 position;
    glm::vec2 velocity;
    float currentSpeed = 0.0f;
    float forwardSpeed = 0.0f;
    float angle = 0.0f;
    float angularVelocity = 0.0f;
  };

  Car(b2BodyId bodyId);
  ~Car() = default;

  void update(const InputState& input);

  CarProperties& getProperties() { return m_properties; }
  const CarProperties& getProperties() const { return m_properties; }
  void setProperties(const CarProperties& props) { m_properties = props; }
  DebugInfo getDebugInfo() const;

private:
  b2BodyId m_bodyId;
  CarProperties m_properties;

  b2Vec2 getForwardVector() const;
  void updateMovement(const InputState& input);
  void handleTurning(const InputState& input, float forwardSpeed);
  void applyDrag(const b2Vec2& currentVel, float forwardSpeed);
};
