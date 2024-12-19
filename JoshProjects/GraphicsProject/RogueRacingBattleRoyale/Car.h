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
#include "PlaceableObject.h"

class ObjectManager;

class Car {
public:
  struct CarProperties {
    // Movement properties
    float maxSpeed = 1000.0f;
    float acceleration = 10000.0f;
    float turnSpeed = 30.0f;
    float lateralDamping = 0.5f;
    float dragFactor = 0.995f;
    float brakingForce = 0.1f;
    float maxAngularVelocity = 4.0f;
    float minSpeedForTurn = 1.0f;
    float turnResetRate = 5.0f;
    // Friction properties
    float wheelFriction = 1.0f;
    float baseFriction = 0.5f;
    float frictionImbalanceSensitivity = 7.5f;
    float surfaceDragSensitivity = 0.8f;
    // Drift properties
    float wheelGrip = 0.49f;
    float driftState = 0.0f;
    float driftDecayRate = 0.35f;
    float lastBackwardDistance = 0.0f;
    bool lastCrossedBackwards = false;
    bool raceStarted = false;
    // Race properties
    int currentLap = 0;
    float lapProgress = 0.0f;
    int racePosition = 0;
    bool lastStartLineSide = false;
    glm::vec2 lastPosition = glm::vec2(0.0f);
    // Booster properties
    float currentBoostSpeed = 0.0f;
    float boostAccumulator = 0.0f;
    bool isOnBooster = false;
    const PlaceableObject* currentBooster = nullptr;
    float boosterMultiplier = 1.0f;
    // Leveling
    int totalXP = 0;

    void reset() {
      totalXP = 0;
    }
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
  void updateStartLineCrossing(const SplineTrack* track);
  void resetPosition(const b2Vec2& position = { -100.0f, -100.0f }, float angle = 0.0f);
  void onSensorEnter(b2BodyId sensorBody);
  void onSensorExit(b2BodyId sensorBody);

  CarProperties& getProperties() { return m_properties; }
  void setProperties(const CarProperties& props) {
    // Ensure XP from previous state is preserved and only incremented
    int previousXP = m_properties.totalXP;
    m_properties = props;
    if (props.totalXP > previousXP) {
      m_properties.totalXP = props.totalXP; // Keep new XP if it's higher
    }
    else {
      m_properties.totalXP = previousXP; // Keep old XP if new is lower/same
    }
  }
  DebugInfo getDebugInfo() const;
  void setColor(const JAGEngine::ColorRGBA8& color) { m_color = color; }
  void setTrack(SplineTrack* track) {
    m_track = track;
    //b2Body_SetUserData(m_bodyId, static_cast<void*>(track));
  }
  void setObjectManager(ObjectManager* manager) { m_objectManager = manager; }

  float getEffectiveFriction() const {
    return m_properties.wheelFriction * m_properties.baseFriction;
  }
  int getCurrentLap() const { return m_properties.currentLap; }

  SplineTrack* getTrack() const { return m_track; }

  float getTotalRaceProgress() const;

  const JAGEngine::ColorRGBA8& getColor() const { return m_color; }

  const std::array<WheelState, 4>& getWheelStates() const { return m_wheelStates; }

  friend class PhysicsSystem;

private:
  static constexpr bool DEBUG_OUTPUT = false;

  static b2Rot angleToRotation(float angle) {
    return b2MakeRot(angle);
  }

  b2BodyId m_bodyId;
  CarProperties m_properties;
  ObjectManager* m_objectManager = nullptr;

  b2Vec2 getForwardVector() const;
  void checkCollisions();
  float calculateLapProgress(const SplineTrack* track);
  void updateMovement(const InputState& input);
  void updateBoostEffects();
  void checkBoosterCollisions(ObjectManager* objectManager);
  void handleBoosterCollision(const PlaceableObject* booster);
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
