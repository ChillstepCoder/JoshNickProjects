// Car.h

#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include "InputState.h"
#include "WheelCollider.h"
#include "SplineTrack.h"
#include "JAGEngine/Vertex.h"
#include <array>
#include <memory>

class Car {
public:
  struct CarProperties {
    // Movement properties
    float maxSpeed = 1000.0f;
    float acceleration = 10000.0f;
    float turnSpeed = 30.0f;
    float lateralDamping = 0.5f;     // Base lateral damping
    float dragFactor = 0.995f;
    float brakingForce = 0.1f;
    float maxAngularVelocity = 4.0f;
    float minSpeedForTurn = 1.0f;
    float turnResetRate = 5.0f;

    // Friction properties
    float wheelFriction = 1.0f;
    float baseFriction = 0.5f;
    float frictionImbalanceSensitivity = 1.0f;  // 0 = no effect, 1 = normal, 2 = double effect
    float surfaceDragSensitivity = 1.0f;

    // Drift properties
    float wheelGrip = 0.49f;           // 0 = max grip (hard to drift), 1 = low grip
    float driftState = 0.0f;          // Current drift amount (0-1)
    float driftDecayRate = 0.35f;      // Base decay rate, modified by wheel grip
  };

  struct DebugInfo {
    glm::vec2 position;
    glm::vec2 velocity;
    float currentSpeed = 0.0f;
    float forwardSpeed = 0.0f;
    float angle = 0.0f;
    float angularVelocity = 0.0f;
    float effectiveFriction = 0.0f;
    b2BodyId bodyId;
  };

  struct WheelState {
    WheelCollider::Surface surface;
    float frictionMultiplier;
    glm::vec2 position;
  };

  Car(b2BodyId bodyId);
  ~Car() = default;

  void update(const InputState& input);
  void resetPosition(const b2Vec2& position = { -100.0f, -100.0f }, float angle = 0.0f);

  CarProperties& getProperties() { return m_properties; }
  const CarProperties& getProperties() const { return m_properties; }
  void setProperties(const CarProperties& props) { m_properties = props; }
  DebugInfo getDebugInfo() const;
  void setColor(const JAGEngine::ColorRGBA8& color) { m_color = color; }
  void setTrack(SplineTrack* track) {
    m_track = track;
    b2Body_SetUserData(m_bodyId, static_cast<void*>(track));
  }

  float getEffectiveFriction() const {
    return m_properties.wheelFriction * m_properties.baseFriction;
  }

  SplineTrack* getTrack() const { return m_track; }

  const JAGEngine::ColorRGBA8& getColor() const { return m_color; }

  const std::array<WheelState, 4>& getWheelStates() const { return m_wheelStates; }

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
  JAGEngine::ColorRGBA8 m_color{ 255, 0, 0, 255 };

  SplineTrack* m_track = nullptr;
  std::array<std::unique_ptr<WheelCollider>, 4> m_wheelColliders;
  std::array<WheelState, 4> m_wheelStates;
  void updateWheelColliders();
  void initializeWheelColliders();
  float calculateAverageWheelFriction() const;
};
