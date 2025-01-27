//PhysicsSystem.h

#pragma once
#include <Box2D/box2d.h>
#include <vector>
#include <memory>
#include "ObjectProperties.h"
#include "PhysicsCategories.h"

class Car;
class PlaceableObject;
class XPPickupObject;
class AudioEngine;

class PhysicsSystem {
public:
  struct CollisionInfo {
    float speed;
    float mass;
    Car* carA;
    Car* carB;
  };

  PhysicsSystem();
  ~PhysicsSystem();

  void init(float gravityX, float gravityY);
  void update(float timeStep);
  void cleanup();

  b2BodyId createStaticBody(float x, float y);
  b2BodyId createDynamicBody(float x, float y);

  void createPillShape(b2BodyId bodyId, float width, float height,
    uint16_t categoryBits, uint16_t maskBits,
    CollisionType collisionType,
    float density = 1.0f, float friction = 0.3f);

  b2ShapeId createCircleShape(b2BodyId bodyId, float radius,
    uint16_t categoryBits, uint16_t maskBits,
    CollisionType collisionType,
    float density = 1.0f, float friction = 0.3f);

  void synchronizeTransforms();

  // Setters
  void setAudioEngine(AudioEngine* audioEngine) { m_audioEngine = audioEngine; }

  // Getters
  b2WorldId getWorld() const { return m_worldId; }



  bool checkIsPowerup(b2BodyId bodyId);

private:
  static constexpr bool DEBUG_OUTPUT = false;

  b2WorldId m_worldId;
  std::vector<b2BodyId> m_dynamicBodies;
  b2ContactBeginTouchEvent m_beginContactEvents[256];
  b2ContactEndTouchEvent m_endContactEvents[256];
  b2ContactHitEvent m_hitContactEvents[256];

  bool isValidBody(b2BodyId bodyId);
  bool isValidUserData(void* userData) const;
  bool isCarBody(b2BodyId bodyId) const;
  bool isObjectBody(b2BodyId bodyId) const;
  void* tryGetUserData(b2BodyId bodyId);

  void handleSensorEvents();
  void handleCollisionEvents();

  static void* enqueueTask(b2TaskCallback* task, int32_t itemCount, int32_t minRange,
    void* taskContext, void* userContext);
  static void finishTask(void* taskPtr, void* userContext);

  AudioEngine* m_audioEngine = nullptr;

};

