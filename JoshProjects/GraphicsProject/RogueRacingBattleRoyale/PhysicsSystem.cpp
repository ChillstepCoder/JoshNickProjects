#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem() : m_worldId(b2_nullWorldId) {
}

PhysicsSystem::~PhysicsSystem() {
  cleanup();
}

void PhysicsSystem::init(float gravityX, float gravityY) {
  // Create the world with default settings
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = { gravityX, gravityY };
  worldDef.enqueueTask = enqueueTask;
  worldDef.finishTask = finishTask;
  worldDef.userTaskContext = this;

  m_worldId = b2CreateWorld(&worldDef);
}

void PhysicsSystem::update(float timeStep) {
  const int32_t velocityIterations = 8;
  const int32_t positionIterations = 3;

  if (b2World_IsValid(m_worldId)) {
    b2World_Step(m_worldId, timeStep, velocityIterations);
  }
}

void PhysicsSystem::cleanup() {
  if (b2World_IsValid(m_worldId)) {
    b2DestroyWorld(m_worldId);
    m_worldId = b2_nullWorldId;
  }
}

b2BodyId PhysicsSystem::createStaticBody(float x, float y) {
  if (!b2World_IsValid(m_worldId)) return b2_nullBodyId;

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.position = { x, y };
  bodyDef.type = b2_staticBody;
  return b2CreateBody(m_worldId, &bodyDef);
}

b2BodyId PhysicsSystem::createDynamicBody(float x, float y) {
  if (!b2World_IsValid(m_worldId)) return b2_nullBodyId;

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.position = { x, y };
  bodyDef.type = b2_dynamicBody;
  return b2CreateBody(m_worldId, &bodyDef);
}

void PhysicsSystem::createBoxShape(b2BodyId bodyId, float width, float height,
  float density, float friction) {
  if (!b2Body_IsValid(bodyId)) return;

  b2Polygon box = b2MakeBox(width * 0.5f, height * 0.5f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = density;
  shapeDef.friction = friction;
  b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

void* PhysicsSystem::enqueueTask(b2TaskCallback* task, int32_t itemCount,
  int32_t minRange, void* taskContext, void* userContext) {
  // Execute task immediately in single-threaded mode
  task(0, itemCount, 0, taskContext);
  return nullptr;
}

void PhysicsSystem::finishTask(void* taskPtr, void* userContext) {
  // Nothing to do in single-threaded mode
}
