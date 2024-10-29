// Car.h
#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include "InputState.h"

class Car {
public:
  struct CarProperties {
    // Movement properties
    float maxSpeed = 557.0f;         // Minimum good value from testing
    float acceleration = 10000.0f;    // Minimum good value
    float turnSpeed = 10.0f;          // Minimum good value
    float lateralDamping = 0.95f;     // Good starting value
    float dragFactor = 0.99f;         // Good starting value
    float turnResetRate = 1.0f;       // Good starting value
    float maxAngularVelocity = 2.7f;  // Good starting value
    float brakingForce = 0.5f;        // Good starting value
    float minSpeedForTurn = 1.0f;     // Good starting value

    // Friction properties
    float wheelFriction = 1.0f;       // Base friction of the car's wheels
    float baseFriction = 0.5f;        // Default surface friction
  };

  struct DebugInfo {
    glm::vec2 position;
    glm::vec2 velocity;
    float currentSpeed = 0.0f;
    float forwardSpeed = 0.0f;
    float angle = 0.0f;
    float angularVelocity = 0.0f;
    float effectiveFriction = 0.0f;   // Combined wheel and surface friction
  };

  Car(b2BodyId bodyId);
  ~Car() = default;

  void update(const InputState& input);
  void resetPosition(const b2Vec2& position = { -100.0f, -100.0f }, float angle = 0.0f);

  CarProperties& getProperties() { return m_properties; }
  const CarProperties& getProperties() const { return m_properties; }
  void setProperties(const CarProperties& props) { m_properties = props; }
  DebugInfo getDebugInfo() const;

  float getEffectiveFriction() const {
    return m_properties.wheelFriction * m_properties.baseFriction;
  }

private:
  static b2Rot angleToRotation(float angle) {
    return b2MakeRot(angle);
  }

  b2BodyId m_bodyId;
  CarProperties m_properties;

  b2Vec2 getForwardVector() const;
  void updateMovement(const InputState& input);
  void handleTurning(const InputState& input, float forwardSpeed);
  void applyDrag(const b2Vec2& currentVel, float forwardSpeed);
  void applyFriction(const b2Vec2& currentVel);
};
