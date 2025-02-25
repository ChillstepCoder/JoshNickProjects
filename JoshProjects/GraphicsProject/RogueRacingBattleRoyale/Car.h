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
#include "ObjectProperties.h"
#include <AK/SoundEngine/Common/AkTypes.h>
#include "IPhysicsUserData.h"

class AudioEngine;
class PlaceableObject;
class ObjectManager;

class Car : public IPhysicsUserData {
public:

  struct CarProperties {
    int highestLapCompleted = 0;

    struct LapBonus {
      float value = 0.0f;      // Current bonus value
      int level = 0;           // Level of this bonus
      bool isDiminishing = false; // Whether this approaches 0 or not
    };

    // Base stat levels
    struct StatLevels {
      int topSpeed = 1;
      int acceleration = 1;
      int weight = 1;
      int wheelGrip = 1;
      int handling = 1;
      int booster = 1;
      int surfaceResistance = 1;
      int damageResistance = 1;
      int xpGain = 1;
      int braking = 1;
    };

    // Per-lap bonuses and special stats
    struct SpecialStats {
      LapBonus topSpeed;
      LapBonus acceleration;
      LapBonus wheelGrip;
      LapBonus handling;
      LapBonus booster;
      LapBonus surfaceResistance;
      LapBonus damageResistance;
      LapBonus xpGain;
      LapBonus braking;
      LapBonus weight;
    };

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
    float weight = 100.0f;
    // Drift properties
    float wheelGrip = 0.49f;
    float driftState = 0.0f;
    float driftDecayRate = 1.00f;
    float lastBackwardDistance = 0.0f;
    bool lastCrossedBackwards = false;
    bool raceStarted = false;
    // Race properties
    int currentLap = 0;
    float lapProgress = 0.0f;
    int racePosition = 0;
    glm::vec2 lastPosition = glm::vec2(0.0f);
    bool finished = false;
    float finishTime = 0.0f; // in seconds
    bool crossedStartLine = false;
    int lastStartLineSide = -1;

    // Booster properties
    float currentBoostSpeed = 0.0f;
    float boostAccumulator = 0.0f;
    bool isOnBooster = false;
    const PlaceableObject* currentBooster = nullptr;
    float boosterMultiplier = 1.0f;
    // Leveling
    int totalXP = 0;
    int xpLevelUpAmount = 10;
    int level = 1;


    StatLevels statLevels;
    SpecialStats specialStats;

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
  void updatePhysicsWeight();
  void resetPosition(const b2Vec2& position = { -100.0f, -100.0f }, float angle = 0.0f);

  CarProperties& getProperties() { return m_properties; }
  void setProperties(const CarProperties& props) {
    m_properties = props;
  }
  DebugInfo getDebugInfo() const;
  void setColor(const JAGEngine::ColorRGBA8& color) { m_color = color; }
  void setTrack(SplineTrack* track) {
    m_track = track;
    //b2Body_SetUserData(m_bodyId, static_cast<void*>(track));
  }
  void setObjectManager(ObjectManager* manager) { m_objectManager = manager; }
  void setAudioEngine(AudioEngine* engine) {
    std::cout << "Setting audio engine for car" << std::endl;
    m_audioEngine = engine;
  }
  void setScrapingBarrier(bool scraping) { m_isScrapingBarrier = scraping; }

  float getEffectiveFriction() const {
    return m_properties.wheelFriction * m_properties.baseFriction;
  }
  int getCurrentLap() const { return m_properties.currentLap; }
  ObjectType getObjectType() const { return ObjectType::Default; }

  SplineTrack* getTrack() const { return m_track; }

  AudioEngine* getAudioEngine() const { return m_audioEngine; }
  AkGameObjectID getAudioId() const { return static_cast<AkGameObjectID>(m_bodyId.index1); }

  float getTotalRaceProgress() const;

  const JAGEngine::ColorRGBA8& getColor() const { return m_color; }

  const std::array<WheelState, 4>& getWheelStates() const { return m_wheelStates; }
  virtual bool isCar() const override { return true; }

  bool isScrapingBarrier() const { return m_isScrapingBarrier; }

  friend class PhysicsSystem;

private:
  static constexpr bool DEBUG_OUTPUT = false;

  static b2Rot angleToRotation(float angle) {
    return b2MakeRot(angle);
  }

  b2BodyId m_bodyId;
  CarProperties m_properties;
  ObjectManager* m_objectManager = nullptr;
  bool m_isScrapingBarrier = false;

  b2Vec2 getForwardVector() const;
  void applyLapBonuses();
  float calculateLapProgress(const SplineTrack* track);
  void updateMovement(const InputState& input);
  void updateBoostEffects();
  //void checkBoosterCollisions(ObjectManager* objectManager);
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

  AudioEngine* m_audioEngine = nullptr;

};
