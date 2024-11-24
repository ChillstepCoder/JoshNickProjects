//PhysicsSystem.h

#pragma once
#include <Box2D/box2d.h>
#include <memory>
#include <vector>

class PhysicsSystem {
public:
  // Collision category bits - based on behavior rather than specific objects
  static const uint16_t CATEGORY_CAR = 0x0001;         // Moving vehicles
  static const uint16_t CATEGORY_BARRIER = 0x0002;     // Immovable world barriers
  static const uint16_t CATEGORY_SOLID = 0x0004;       // Solid obstacles (trees, poles, etc)
  static const uint16_t CATEGORY_HAZARD = 0x0008;      // Drivable hazards (potholes, oil slicks, etc)
  static const uint16_t CATEGORY_PUSHABLE = 0x0010;    // Light objects cars can push (cones, boxes, etc)

  static const uint16_t MASK_CAR = CATEGORY_BARRIER | CATEGORY_SOLID | CATEGORY_PUSHABLE | CATEGORY_CAR;

  // Objects can have different collision behaviors
  enum class CollisionType {
    DEFAULT,      // Normal physics collision
    HAZARD,       // Trigger-style collision for hazards
    PUSHABLE      // Lightweight collision for pushable objects
  };


  PhysicsSystem();
  ~PhysicsSystem();

  void init(float gravityX = 0.0f, float gravityY = -9.81f);
  void update(float timeStep);
  void cleanup();

  // Body creation helpers
  b2BodyId createStaticBody(float x, float y);
  b2BodyId createDynamicBody(float x, float y);
  // Update shape creation to handle different collision types
  void createPillShape(b2BodyId bodyId, float width, float height,
    uint16_t categoryBits, uint16_t maskBits,
    CollisionType collisionType = CollisionType::DEFAULT,
    float density = 1.0f, float friction = 0.3f);

  // Helper to create circular collision shapes for objects
  b2ShapeId createCircleShape(b2BodyId bodyId, float radius,
    uint16_t categoryBits, uint16_t maskBits,
    CollisionType collisionType = CollisionType::DEFAULT,
    float density = 1.0f, float friction = 0.3f);

  b2WorldId getWorld() const { return m_worldId; }

private:
  b2WorldId m_worldId;
  std::vector<b2BodyId> m_dynamicBodies;

  static void* enqueueTask(b2TaskCallback* task, int32_t itemCount,
    int32_t minRange, void* taskContext, void* userContext);
  static void finishTask(void* taskPtr, void* userContext);
  void synchronizeTransforms();

  float m_timeStep = 1.0f / 60.0f;  // Fixed timestep
  float m_minTimeStep = 1.0f / 600.0f;  // Minimum allowed timestep
  float m_maxTimeStep = 1.0f / 30.0f;
};
