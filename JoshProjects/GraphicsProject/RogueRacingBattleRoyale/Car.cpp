// Car.cpp
#include "Car.h"
#include <cmath>
#include <algorithm>

namespace {
  float clamp(float value, float min, float max) {
    return std::min(max, std::max(min, value));
  }

  float b2Vec2Length(const b2Vec2& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
  }
}

Car::Car(b2BodyId bodyId) : m_bodyId(bodyId) {
}

void Car::update(const InputState& input) {
  if (!b2Body_IsValid(m_bodyId)) return;

  b2Vec2 currentVel = b2Body_GetLinearVelocity(m_bodyId);
  float currentSpeed = b2Vec2Length(currentVel);

  // Get forward direction and speed
  b2Vec2 forwardDir = getForwardVector();
  float forwardSpeed = currentVel.x * forwardDir.x + currentVel.y * forwardDir.y;

  // Update movement and turning
  updateMovement(input);
  handleTurning(input, forwardSpeed);
  applyDrag(currentVel, forwardSpeed);
}

b2Vec2 Car::getForwardVector() const {
  float angle = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
  return { std::cos(angle), std::sin(angle) };
}

void Car::updateMovement(const InputState& input) {
  b2Vec2 currentVel = b2Body_GetLinearVelocity(m_bodyId);
  float currentSpeed = b2Vec2Length(currentVel);
  b2Vec2 forwardDir = getForwardVector();

  if (input.accelerating && currentSpeed < m_properties.maxSpeed) {
    b2Vec2 force = {
        forwardDir.x * m_properties.acceleration,
        forwardDir.y * m_properties.acceleration
    };
    b2Body_ApplyForceToCenter(m_bodyId, force, true);
  }

  if (input.braking) {
    float forwardSpeed = currentVel.x * forwardDir.x + currentVel.y * forwardDir.y;
    if (forwardSpeed > m_properties.minSpeedForTurn) {
      b2Vec2 force = {
          -forwardDir.x * m_properties.acceleration * m_properties.brakingForce,
          -forwardDir.y * m_properties.acceleration * m_properties.brakingForce
      };
      b2Body_ApplyForceToCenter(m_bodyId, force, true);
    }
  }
}

void Car::handleTurning(const InputState& input, float forwardSpeed) {
  if (std::abs(forwardSpeed) < m_properties.minSpeedForTurn) {
    b2Body_SetAngularVelocity(m_bodyId, 0.0f);
    return;
  }

  float currentAngularVel = b2Body_GetAngularVelocity(m_bodyId);
  float targetAngularVel = 0.0f;
  float turnFactor = m_properties.turnSpeed * (forwardSpeed / m_properties.maxSpeed);

  if (input.turningLeft) {
    targetAngularVel = turnFactor;
  }
  else if (input.turningRight) {
    targetAngularVel = -turnFactor;
  }

  float newAngularVel;
  if (std::abs(targetAngularVel) > 0.001f) {
    newAngularVel = currentAngularVel + (targetAngularVel - currentAngularVel) * 0.1f;
  }
  else {
    newAngularVel = currentAngularVel * m_properties.turnResetRate;
  }

  newAngularVel = clamp(newAngularVel, -m_properties.maxAngularVelocity, m_properties.maxAngularVelocity);
  b2Body_SetAngularVelocity(m_bodyId, newAngularVel);
}

void Car::applyDrag(const b2Vec2& currentVel, float forwardSpeed) {
  b2Vec2 forwardDir = getForwardVector();

  // Calculate forward and lateral velocities
  b2Vec2 forwardVel = {
      forwardDir.x * forwardSpeed,
      forwardDir.y * forwardSpeed
  };

  b2Vec2 lateralVel = {
      currentVel.x - forwardVel.x,
      currentVel.y - forwardVel.y
  };

  // Apply lateral damping and drag
  b2Vec2 newVel = {
      (forwardVel.x + lateralVel.x * m_properties.lateralDamping) * m_properties.dragFactor,
      (forwardVel.y + lateralVel.y * m_properties.lateralDamping) * m_properties.dragFactor
  };

  b2Body_SetLinearVelocity(m_bodyId, newVel);
}

Car::DebugInfo Car::getDebugInfo() const {
  DebugInfo info;
  b2Vec2 pos = b2Body_GetPosition(m_bodyId);
  b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

  info.position = glm::vec2(pos.x, pos.y);
  info.velocity = glm::vec2(vel.x, vel.y);
  info.currentSpeed = b2Vec2Length(vel);
  info.angle = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
  info.angularVelocity = b2Body_GetAngularVelocity(m_bodyId);

  b2Vec2 forwardDir = getForwardVector();
  info.forwardSpeed = vel.x * forwardDir.x + vel.y * forwardDir.y;

  return info;
}
