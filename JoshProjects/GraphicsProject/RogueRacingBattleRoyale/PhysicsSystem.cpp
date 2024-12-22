//PhysicsSystem.cpp

#include "PhysicsSystem.h"
#include "Car.h"
#include "PlaceableObject.h"
#include <iostream>
#include <cmath>
#include <algorithm>

PhysicsSystem::PhysicsSystem() : m_worldId(b2_nullWorldId) {
}

PhysicsSystem::~PhysicsSystem() {
  cleanup();
}

void PhysicsSystem::init(float gravityX, float gravityY) {
  // Create world with default settings
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = { gravityX, gravityY };
  worldDef.enqueueTask = enqueueTask;
  worldDef.finishTask = finishTask;
  worldDef.userTaskContext = this;
  worldDef.enableContinuous = true;
  worldDef.restitutionThreshold = 0.5f;
  worldDef.contactPushoutVelocity = 3.0f;
  worldDef.enableSleep = false;

  m_worldId = b2CreateWorld(&worldDef);
}

void PhysicsSystem::update(float timeStep) {
  if (!b2World_IsValid(m_worldId)) return;

  // Clean invalid bodies first
  auto it = m_dynamicBodies.begin();
  while (it != m_dynamicBodies.end()) {
    if (!isValidBody(*it)) {
      std::cout << "Removing invalid body: " << it->index1 << std::endl;
      it = m_dynamicBodies.erase(it);
    }
    else {
      ++it;
    }
  }

  // Now process the remaining valid bodies
  for (b2BodyId carBody : m_dynamicBodies) {
    void* userData = tryGetUserData(carBody);
    if (!userData) continue;

    Car* car = static_cast<Car*>(userData);
    findOverlappingBodies(carBody, car);
  }

  // Physics step
  const float fixedTimeStep = 1.0f / 60.0f;
  const int subStepCount = static_cast<int>(std::ceil(timeStep / fixedTimeStep));
  for (int i = 0; i < subStepCount; i++) {
    b2World_Step(m_worldId, fixedTimeStep, 1);
  }
}

void PhysicsSystem::findOverlappingBodies(b2BodyId carBody, Car* car) {
  if (!car || !b2Body_IsValid(carBody)) return;

  b2Vec2 carPos = b2Body_GetPosition(carBody);
  float detectionRadius = 15.0f;

  for (b2BodyId otherId : m_dynamicBodies) {
    if (otherId.index1 == carBody.index1) continue;
    if (!b2Body_IsValid(otherId)) continue;

    void* userData = tryGetUserData(otherId); // Use safe getter
    if (!userData) continue;

    // First check if it's a Car object
    Car* otherCar = dynamic_cast<Car*>(static_cast<Car*>(userData));
    if (otherCar) continue;

    // If not a car, must be a PlaceableObject
    PlaceableObject* obj = static_cast<PlaceableObject*>(userData);
    if (!obj || !obj->isDetectable()) continue;

    // Calculate distance
    b2Vec2 otherPos = b2Body_GetPosition(otherId);
    b2Vec2 diff = { otherPos.x - carPos.x, otherPos.y - carPos.y };
    float distSq = diff.x * diff.x + diff.y * diff.y;

    if (distSq < detectionRadius * detectionRadius) {
      if (obj->isXPPickup()) {
        std::cout << "XP Pickup overlap detected at distance: " << sqrt(distSq) << std::endl;
        std::cout << "Car position: " << carPos.x << "," << carPos.y << std::endl;
        std::cout << "XP position: " << otherPos.x << "," << otherPos.y << std::endl;
      }
      obj->onCarCollision(car);
    }
  }
}

void PhysicsSystem::cleanup() {
  if (b2World_IsValid(m_worldId)) {
    b2DestroyWorld(m_worldId);
    m_worldId = b2_nullWorldId;
  }
  m_dynamicBodies.clear();
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
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = { x, y };
  bodyDef.linearDamping = 0.0f;
  bodyDef.angularDamping = 0.0f;
  bodyDef.isBullet = true;

  b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
  if (b2Body_IsValid(bodyId)) {
    // Check if body already exists in list
    bool bodyExists = false;
    for (const auto& existingBody : m_dynamicBodies) {
      if (existingBody.index1 == bodyId.index1) {
        bodyExists = true;
        std::cout << "WARNING: Body " << bodyId.index1 << " already in tracking list!" << std::endl;
        break;
      }
    }

    if (!bodyExists) {
      std::cout << "Adding new dynamic body " << bodyId.index1 << " to tracking list" << std::endl;
      m_dynamicBodies.push_back(bodyId);
    }
  }
  return bodyId;
}

void PhysicsSystem::createPillShape(b2BodyId bodyId, float width, float height,
  uint16_t categoryBits, uint16_t maskBits,
  CollisionType collisionType,
  float density, float friction) {

  if (!b2Body_IsValid(bodyId)) return;

  b2ShapeDef shapeDef = b2DefaultShapeDef();

  switch (collisionType) {
  case CollisionType::HAZARD:
  case CollisionType::POWERUP:
    shapeDef.isSensor = true;
    shapeDef.density = 0.0f;
    shapeDef.friction = 0.0f;
    break;
  case CollisionType::PUSHABLE:
    shapeDef.density = 0.2f;
    shapeDef.friction = 0.4f;
    shapeDef.restitution = 0.2f;
    break;
  default:
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = 0.1f;
    break;
  }

  shapeDef.filter.categoryBits = categoryBits;
  shapeDef.filter.maskBits = maskBits;
  shapeDef.filter.groupIndex = 0;

  float bodyWidth = width * 0.6f;
  float bodyHeight = height * 0.5f;
  float radius = bodyHeight * 0.5f;

  // Central box
  b2Polygon mainBody = b2MakeBox(bodyWidth * 0.5f, bodyHeight * 0.5f);
  b2CreatePolygonShape(bodyId, &shapeDef, &mainBody);

  // Front and back caps
  const int numCapVertices = 8;
  b2Vec2 frontCapVertices[b2_maxPolygonVertices];
  b2Vec2 backCapVertices[b2_maxPolygonVertices];

  for (int i = 0; i < numCapVertices; ++i) {
    float angle = -b2_pi * 0.5f + (float)i / (numCapVertices - 1) * b2_pi;
    frontCapVertices[i].x = bodyWidth * 0.5f + cosf(angle) * radius;
    frontCapVertices[i].y = sinf(angle) * radius;
  }

  for (int i = 0; i < numCapVertices; ++i) {
    float angle = b2_pi * 0.5f + (float)i / (numCapVertices - 1) * b2_pi;
    backCapVertices[i].x = -bodyWidth * 0.5f + cosf(angle) * radius;
    backCapVertices[i].y = sinf(angle) * radius;
  }

  b2Hull frontCapHull;
  frontCapHull.count = numCapVertices;
  for (int i = 0; i < numCapVertices; ++i) {
    frontCapHull.points[i] = frontCapVertices[i];
  }

  b2Hull backCapHull;
  backCapHull.count = numCapVertices;
  for (int i = 0; i < numCapVertices; ++i) {
    backCapHull.points[i] = backCapVertices[i];
  }

  b2Polygon frontCap = b2MakePolygon(&frontCapHull, 0.0f);
  b2Polygon backCap = b2MakePolygon(&backCapHull, 0.0f);

  b2CreatePolygonShape(bodyId, &shapeDef, &frontCap);
  b2CreatePolygonShape(bodyId, &shapeDef, &backCap);
}

b2ShapeId PhysicsSystem::createCircleShape(b2BodyId bodyId, float radius,
  uint16_t categoryBits, uint16_t maskBits,
  CollisionType collisionType,
  float density, float friction) {

  std::cout << "\nCreating circle shape at:"
    << "\nRadius: " << radius
    << "\nCategory: " << categoryBits
    << "\nMask: " << maskBits
    << "\nCollision Type: " << static_cast<int>(collisionType)
    << "\nDensity: " << density
    << "\nFriction: " << friction << std::endl;

  if (!b2Body_IsValid(bodyId)) return b2_nullShapeId;

  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.filter.categoryBits = categoryBits;
  shapeDef.filter.maskBits = maskBits;

  switch (collisionType) {
  case CollisionType::POWERUP:
    shapeDef.isSensor = true;
    shapeDef.enableSensorEvents = true;
    shapeDef.enableContactEvents = true;
    shapeDef.enableHitEvents = true;
    break;
  case CollisionType::PUSHABLE:
    shapeDef.density = 0.2f;
    shapeDef.friction = 0.4f;
    shapeDef.restitution = 0.2f;
    break;
  default:
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = 0.1f;
    break;
  }

  b2Circle circle;
  circle.radius = radius;
  circle.center = b2Vec2_zero;

  return b2CreateCircleShape(bodyId, &shapeDef, &circle);
}

bool PhysicsSystem::checkIsPowerup(b2BodyId bodyId) {
  if (!b2Body_IsValid(bodyId)) return false;

  void* userData = b2Body_GetUserData(bodyId);
  if (!userData) return false;

  PlaceableObject* obj = static_cast<PlaceableObject*>(userData);
  return obj && (obj->isBooster() || obj->isXPPickup());
}

void* PhysicsSystem::enqueueTask(b2TaskCallback* task, int32_t itemCount,
  int32_t minRange, void* taskContext, void* userContext) {
  task(0, itemCount, 0, taskContext);
  return nullptr;
}

bool PhysicsSystem::isValidBody(b2BodyId bodyId) {
  if (!b2Body_IsValid(bodyId)) {
    return false;
  }
  void* userData = tryGetUserData(bodyId);
  return userData != nullptr && reinterpret_cast<uintptr_t>(userData) >= 1000;
}

void* PhysicsSystem::tryGetUserData(b2BodyId bodyId) {
  if (!b2Body_IsValid(bodyId)) {
    std::cout << "Body " << bodyId.index1 << " is invalid" << std::endl;
    return nullptr;
  }

  try {
    void* userData = b2Body_GetUserData(bodyId);
    if (!userData) {
      std::cout << "No user data for body " << bodyId.index1 << std::endl;
    }
    return userData;
  }
  catch (...) {
    std::cout << "Exception getting user data for body " << bodyId.index1 << std::endl;
    return nullptr;
  }
}

void PhysicsSystem::finishTask(void* taskPtr, void* userContext) {
  // Nothing to do in single-threaded mode
}

void PhysicsSystem::synchronizeTransforms() {
  if (!b2World_IsValid(m_worldId)) return;

  // Remove invalid bodies to prevent access violations
  m_dynamicBodies.erase(
    std::remove_if(m_dynamicBodies.begin(), m_dynamicBodies.end(),
      [](b2BodyId bodyId) { return !b2Body_IsValid(bodyId); }),
    m_dynamicBodies.end()
  );
}
