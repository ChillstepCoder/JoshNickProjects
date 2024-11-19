//PhysicsSystem.cpp

#include "PhysicsSystem.h"
#include <iostream>
#include <cmath> 
#include <algorithm> 

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

void PhysicsSystem::createPillShape(b2BodyId bodyId, float width, float height, float density, float friction) {
  if (!b2Body_IsValid(bodyId)) return;

  // Define the central rectangle (main body)
  float bodyWidth = width * 0.6f;
  float bodyHeight = height * 0.5f; // Adjusted to match the height of the car

  b2Polygon mainBody = b2MakeBox(bodyWidth * 0.5f, bodyHeight * 0.5f);

  // Define the shape
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = 0.1f;
    shapeDef.isSensor = false;

    // Set collision filtering for the car
    shapeDef.filter.categoryBits = 0x0001; // Car category
    shapeDef.filter.maskBits = 0x0002;     // Collide with barrier category

  // Corrected variable type and validity check
  b2ShapeId mainBodyShapeId = b2CreatePolygonShape(bodyId, &shapeDef, &mainBody);
  if (!b2Shape_IsValid(mainBodyShapeId)) {
    std::cerr << "Failed to create main body shape.\n";
  }

  // Define the front and back semi-circular caps
  const int numVertices = 8; // Adjust for smoothness
  b2Vec2 frontCapVertices[b2_maxPolygonVertices];
  b2Vec2 backCapVertices[b2_maxPolygonVertices];
  float radius = bodyHeight * 0.5f;

  // Front cap (semi-circle)
  for (int i = 0; i < numVertices; ++i) {
    float angle = (float)i / (numVertices - 1) * b2_pi;
    frontCapVertices[i].x = bodyWidth * 0.5f + cosf(angle) * radius;
    frontCapVertices[i].y = sinf(angle) * radius;
  }

  // Create front cap polygon using b2Hull
  b2Hull frontCapHull;
  frontCapHull.count = numVertices;
  for (int i = 0; i < numVertices; ++i) {
    frontCapHull.points[i] = frontCapVertices[i];
  }

  b2Polygon frontCap = b2MakePolygon(&frontCapHull, 0.0f); // Added radius parameter

  b2ShapeId frontCapShapeId = b2CreatePolygonShape(bodyId, &shapeDef, &frontCap);
  if (!b2Shape_IsValid(frontCapShapeId)) {
    std::cerr << "Failed to create front cap shape.\n";
  }

  // Back cap (semi-circle)
  for (int i = 0; i < numVertices; ++i) {
    float angle = b2_pi + (float)i / (numVertices - 1) * b2_pi;
    backCapVertices[i].x = -bodyWidth * 0.5f + cosf(angle) * radius;
    backCapVertices[i].y = sinf(angle) * radius;
  }

  // Create back cap polygon using b2Hull
  b2Hull backCapHull;
  backCapHull.count = numVertices;
  for (int i = 0; i < numVertices; ++i) {
    backCapHull.points[i] = backCapVertices[i];
  }

  b2Polygon backCap = b2MakePolygon(&backCapHull, 0.0f); // Added radius parameter

  b2ShapeId backCapShapeId = b2CreatePolygonShape(bodyId, &shapeDef, &backCap);
  if (!b2Shape_IsValid(backCapShapeId)) {
    std::cerr << "Failed to create back cap shape.\n";
  }

  // Set damping for the car body
  b2Body_SetLinearDamping(bodyId, 0.5f);
  b2Body_SetAngularDamping(bodyId, 1.0f);
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
