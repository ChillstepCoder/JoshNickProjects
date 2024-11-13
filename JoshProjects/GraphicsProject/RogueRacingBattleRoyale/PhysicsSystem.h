//PhysicsSystem.h

#pragma once
#include <Box2D/box2d.h>
#include <memory>
#include <vector>

class PhysicsSystem {
public:
  PhysicsSystem();
  ~PhysicsSystem();

  void init(float gravityX = 0.0f, float gravityY = -9.81f);
  void update(float timeStep);
  void cleanup();

  // Body creation helpers
  b2BodyId createStaticBody(float x, float y);
  b2BodyId createDynamicBody(float x, float y);
  void createPillShape(b2BodyId bodyId, float width, float height,
    float density = 1.0f, float friction = 0.3f);

  b2WorldId getWorld() const { return m_worldId; }

private:
  b2WorldId m_worldId;
  static void* enqueueTask(b2TaskCallback* task, int32_t itemCount,
    int32_t minRange, void* taskContext, void* userContext);
  static void finishTask(void* taskPtr, void* userContext);
};
