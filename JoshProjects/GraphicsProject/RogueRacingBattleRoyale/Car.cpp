// Car.cpp
#include "Car.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace {
  float clamp(float value, float min, float max) {
    return std::min(max, std::max(min, value));
  }

  float b2Vec2Length(const b2Vec2& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
  }
}

Car::Car(b2BodyId bodyId) : m_bodyId(bodyId) {
  initializeWheelColliders();
}

void Car::update(const InputState& input) {
  if (!b2Body_IsValid(m_bodyId)) return;

  updateWheelColliders();

  b2Vec2 currentVel = b2Body_GetLinearVelocity(m_bodyId);
  float currentSpeed = b2Vec2Length(currentVel);
  b2Vec2 forwardDir = getForwardVector();
  float forwardSpeed = currentVel.x * forwardDir.x + currentVel.y * forwardDir.y;

  // Update drift state
  if (input.braking && (input.turningLeft || input.turningRight)) {
    float speedThreshold = 75.0f;
    float speedRatio = currentSpeed / speedThreshold;
    m_properties.driftState = glm::clamp(speedRatio, 0.0f, 1.0f);
  }
  else {
    float decayRate = m_properties.driftDecayRate * (1.0f - m_properties.wheelGrip);
    m_properties.driftState = std::max(0.0f, m_properties.driftState - decayRate * (1.0f / 60.0f));
  }

  // Map drift state directly to lateral damping: 0->0.5, 1->1.0
  m_properties.lateralDamping = m_properties.driftState * 0.5f + 0.5f;

  updateMovement(input);
  handleTurning(input, forwardSpeed);
  applyDrag(currentVel, forwardSpeed);
  applyFriction(currentVel);
}

void Car::applyFriction(const b2Vec2& currentVel) {
  float averageWheelFriction = calculateAverageWheelFriction();
  float effectiveFriction = m_properties.wheelFriction * m_properties.baseFriction * averageWheelFriction;

  // Apply friction as a force opposing current velocity
  if (b2Vec2Length(currentVel) > 0.1f) {
    b2Vec2 frictionForce = {
        -currentVel.x * effectiveFriction,
        -currentVel.y * effectiveFriction
    };
    b2Body_ApplyForceToCenter(m_bodyId, frictionForce, true);
  }
}

b2Vec2 Car::getForwardVector() const {
  float angle = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
  return { std::cos(angle), std::sin(angle) };
}

void Car::updateMovement(const InputState& input) {
  b2Vec2 currentVel = b2Body_GetLinearVelocity(m_bodyId);
  float currentSpeed = b2Vec2Length(currentVel);
  b2Vec2 forwardDir = getForwardVector();
  float forwardSpeed = currentVel.x * forwardDir.x + currentVel.y * forwardDir.y;

  // Reset forces if no input
  if (!input.accelerating && !input.braking) {
    // Let natural drag and friction slow the car down
    return;
  }

  // Forward acceleration
  if (input.accelerating && currentSpeed < m_properties.maxSpeed * 0.05f ) {
    b2Vec2 force = {
        forwardDir.x * m_properties.acceleration,
        forwardDir.y * m_properties.acceleration
    };
    b2Body_ApplyForceToCenter(m_bodyId, force, true);
  }

  // Braking/Reverse
  if (input.braking) {
    float brakeForce = m_properties.acceleration * m_properties.brakingForce;

    if (forwardSpeed > 0.1f) {
      // Moving forward - apply brakes
      b2Vec2 force = {
          -forwardDir.x * brakeForce,
          -forwardDir.y * brakeForce
      };
      b2Body_ApplyForceToCenter(m_bodyId, force, true);
    }
    else if (forwardSpeed > -m_properties.maxSpeed * 0.5f) {
      // Moving slow enough or backwards - apply reverse thrust (half of forward max speed)
      b2Vec2 force = {
          -forwardDir.x * brakeForce * 0.5f,
          -forwardDir.y * brakeForce * 0.5f
      };
      b2Body_ApplyForceToCenter(m_bodyId, force, true);
    }
  }
}

void Car::handleTurning(const InputState& input, float forwardSpeed) {
  float currentAngularVel = b2Body_GetAngularVelocity(m_bodyId);
  float absForwardSpeed = std::abs(forwardSpeed);

  // Strong reset when nearly stopped
  if (absForwardSpeed < m_properties.minSpeedForTurn) {
    b2Body_SetAngularVelocity(m_bodyId, 0.0f);
    return;
  }

  // Calculate base turn rate based on speed
  float turnFactor = (m_properties.turnSpeed / ((1.0f + pow(absForwardSpeed / (10.0f),1.35f)))*(absForwardSpeed/ (150.0f)))*(1.0f+m_properties.driftState * 1.3f);

  // Reverse steering direction when going backwards
  if (forwardSpeed < 0) {
    turnFactor = -turnFactor;
  }

  float targetAngularVel = 0.0f;
  float correctionRate = 0.2f; // Base correction rate

  // Determine steering direction and correction rate
  if (input.turningLeft) {
    targetAngularVel = turnFactor;
    // If we're turning hard right, apply stronger correction to break out
    if (currentAngularVel < -m_properties.maxAngularVelocity * 0.5f) {
      correctionRate = 0.4f; // Stronger correction when counter-steering
    }
  }
  else if (input.turningRight) {
    targetAngularVel = -turnFactor;
    // If we're turning hard left, apply stronger correction to break out
    if (currentAngularVel > m_properties.maxAngularVelocity * 0.5f) {
      correctionRate = 0.4f; // Stronger correction when counter-steering
    }
  }
  else {
    // No input - return to center with moderate force
    targetAngularVel = 0.0f;
    correctionRate = 0.3f; // Moderate correction when returning to center
  }

  // Apply steering with variable correction rate
  float newAngularVel = currentAngularVel + (targetAngularVel - currentAngularVel) * correctionRate;

  // Add extra correction force when trying to break out of a turn
  if ((input.turningRight && currentAngularVel > 0.0f) ||
    (input.turningLeft && currentAngularVel < 0.0f)) {
    // Apply additional counter-force
    newAngularVel += (targetAngularVel - currentAngularVel) * 0.3f;
  }

  // Hard limits to prevent getting stuck
  newAngularVel = clamp(newAngularVel, -m_properties.maxAngularVelocity, m_properties.maxAngularVelocity);

  // Apply additional damping when changing directions
  if ((newAngularVel > 0 && currentAngularVel < 0) ||
    (newAngularVel < 0 && currentAngularVel > 0)) {
    newAngularVel *= 1.2f; // Boost direction changes
  }

  b2Body_SetAngularVelocity(m_bodyId, newAngularVel);

  //// Debug output
  //std::cout << "Steering - "
  //  << "Current: " << currentAngularVel
  //  << " Target: " << targetAngularVel
  //  << " New: " << newAngularVel
  //  << " Correction: " << correctionRate
  //  << "\n";
}


void Car::resetPosition(const b2Vec2& position, float angle) {
  if (!b2Body_IsValid(m_bodyId)) return;

  // Create rotation from angle
  b2Rot rotation = b2MakeRot(angle);  // This creates a proper b2Rot object from the angle

  b2Body_SetTransform(m_bodyId, position, rotation);
  b2Body_SetLinearVelocity(m_bodyId, { 0.0f, 0.0f });
  b2Body_SetAngularVelocity(m_bodyId, 0.0f);
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

  float lateralSpeed = b2Vec2Length(lateralVel);

  // Stabilize lateral velocity to reduce bouncing
  if (lateralSpeed > m_properties.maxSpeed * 0.1f) {
    lateralVel.x *= 0.7f;  // Reduce lateral velocity
    lateralVel.y *= 0.7f;
  }

  // Apply damping to avoid sticking to the barrier
  b2Vec2 newVel = {
      (forwardVel.x + lateralVel.x * m_properties.lateralDamping) * m_properties.dragFactor,
      (forwardVel.y + lateralVel.y * m_properties.lateralDamping) * m_properties.dragFactor
  };

  // Apply small impulse if near-zero velocity
  if (b2Vec2Length(newVel) < 0.5f) {
    b2Vec2 impulse = {
        forwardDir.x * m_properties.acceleration * 0.02f,
        forwardDir.y * m_properties.acceleration * 0.02f
    };
    b2Body_ApplyLinearImpulseToCenter(m_bodyId, impulse, true);
  }

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
  info.effectiveFriction = getEffectiveFriction();
  info.bodyId = m_bodyId;

  return info;
}

void Car::initializeWheelColliders() {
  float wheelBaseWidth = 14.0f;
  float wheelBaseLength = 7.0f;
  float wheelWidth = 6.0f;
  float wheelHeight = 3.0f;

  // Create wheel colliders in the correct order to match the debug output
  WheelCollider::Config wheelConfigs[4] = {
    // Back Left (index 0 should show as Front Left)
    { wheelWidth, wheelHeight, glm::vec2(-wheelBaseWidth / 2, -wheelBaseLength / 2) },
    // Back Right (index 1 should show as Front Right)
    { wheelWidth, wheelHeight, glm::vec2(wheelBaseWidth / 2, -wheelBaseLength / 2) },
    // Front Left (index 2 should show as Back Left)
    { wheelWidth, wheelHeight, glm::vec2(-wheelBaseWidth / 2, wheelBaseLength / 2) },
    // Front Right (index 3 should show as Back Right)
    { wheelWidth, wheelHeight, glm::vec2(wheelBaseWidth / 2, wheelBaseLength / 2) }
  };

  // Create wheel colliders in this order
  for (int i = 0; i < 4; i++) {
    m_wheelColliders[i] = std::make_unique<WheelCollider>(m_bodyId, wheelConfigs[i]);
  }
}

void Car::updateWheelColliders() {
  // Reorder the wheel states to match the expected debug output order
  const int wheelOrder[4] = { 3, 1, 2, 0 };  // Maps physical wheels to debug output order

  // Update each wheel collider
  for (size_t i = 0; i < m_wheelColliders.size(); i++) {
    m_wheelColliders[i]->update();

    // Store states in the order expected by the debug output
    int debugIndex = wheelOrder[i];
    m_wheelStates[debugIndex].surface = m_wheelColliders[i]->getSurface();
    m_wheelStates[debugIndex].frictionMultiplier = m_wheelColliders[i]->getFrictionMultiplier();
    m_wheelStates[debugIndex].position = m_wheelColliders[i]->getPosition();
  }
}

float Car::calculateAverageWheelFriction() const {
  float totalFriction = 0.0f;
  for (const auto& state : m_wheelStates) {
    totalFriction += state.frictionMultiplier;
  }
  return totalFriction / 4.0f;
}
