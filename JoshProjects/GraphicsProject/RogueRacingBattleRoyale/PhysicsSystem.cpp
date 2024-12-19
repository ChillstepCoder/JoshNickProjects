//PhysicsSystem.cpp

#include "PhysicsSystem.h"
#include "Car.h"
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

  // Check collisions between cars and powerups
  for (b2BodyId carBody : m_dynamicBodies) {
    if (!b2Body_IsValid(carBody)) continue;

    // Get car position and user data
    b2Vec2 carPos = b2Body_GetPosition(carBody);
    void* userData = b2Body_GetUserData(carBody);
    Car* car = static_cast<Car*>(userData);
    if (!car) continue;

    findOverlappingBodies(carBody, car);
  }

  // Normal physics step
  const float fixedTimeStep = 1.0f / 60.0f;
  const int subStepCount = static_cast<int>(std::ceil(timeStep / fixedTimeStep));
  for (int i = 0; i < subStepCount; i++) {
    b2World_Step(m_worldId, fixedTimeStep, 1);
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
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = { x, y };
  bodyDef.linearDamping = 0.0f;
  bodyDef.angularDamping = 0.0f;
  bodyDef.isBullet = true;

  b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
  if (b2Body_IsValid(bodyId)) {
    m_dynamicBodies.push_back(bodyId);
  }
  return bodyId;
}

void PhysicsSystem::createPillShape(b2BodyId bodyId, float width, float height,
  uint16_t categoryBits, uint16_t maskBits,
  CollisionType collisionType,
  float density, float friction) {

  if (!b2Body_IsValid(bodyId)) return;

  std::cout << "\nCreating pill shape physics:\n";
  std::cout << "Category bits: " << categoryBits << "\n";
  std::cout << "Mask bits: " << maskBits << "\n";
  std::cout << "Collision type: " << static_cast<int>(collisionType) << "\n";

  // Central rectangle (main body)
  float bodyWidth = width * 0.6f;
  float bodyHeight = height * 0.5f;
  float radius = bodyHeight * 0.5f;

  b2ShapeDef shapeDef = b2DefaultShapeDef();

  // Configure physics based on collision type
  switch (collisionType) {
  case CollisionType::HAZARD:
  case CollisionType::POWERUP:  // Handle powerups like hazards for collision
    shapeDef.isSensor = true;
    shapeDef.density = 0.0f;
    shapeDef.friction = 0.0f;
    break;

  case CollisionType::PUSHABLE:
    shapeDef.density = 0.2f;
    shapeDef.friction = 0.4f;
    shapeDef.restitution = 0.2f;
    break;

  default:  // DEFAULT type
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = 0.1f;
    break;
  }

  // Set collision filtering
  shapeDef.filter.categoryBits = categoryBits;
  shapeDef.filter.maskBits = maskBits;

  // Create central box
  b2Polygon mainBody = b2MakeBox(bodyWidth * 0.5f, bodyHeight * 0.5f);
  b2CreatePolygonShape(bodyId, &shapeDef, &mainBody);

  // Create front and back capsules
  const int numCapVertices = 8;
  b2Vec2 frontCapVertices[b2_maxPolygonVertices];
  b2Vec2 backCapVertices[b2_maxPolygonVertices];

  // Create front cap
  for (int i = 0; i < numCapVertices; ++i) {
    float angle = -b2_pi * 0.5f + (float)i / (numCapVertices - 1) * b2_pi;
    frontCapVertices[i].x = bodyWidth * 0.5f + cosf(angle) * radius;
    frontCapVertices[i].y = sinf(angle) * radius;
  }

  // Create back cap
  for (int i = 0; i < numCapVertices; ++i) {
    float angle = b2_pi * 0.5f + (float)i / (numCapVertices - 1) * b2_pi;
    backCapVertices[i].x = -bodyWidth * 0.5f + cosf(angle) * radius;
    backCapVertices[i].y = sinf(angle) * radius;
  }

  // Create hull for front cap
  b2Hull frontCapHull;
  frontCapHull.count = numCapVertices;
  for (int i = 0; i < numCapVertices; ++i) {
    frontCapHull.points[i] = frontCapVertices[i];
  }

  // Create hull for back cap
  b2Hull backCapHull;
  backCapHull.count = numCapVertices;
  for (int i = 0; i < numCapVertices; ++i) {
    backCapHull.points[i] = backCapVertices[i];
  }

  // Create cap polygons
  b2Polygon frontCap = b2MakePolygon(&frontCapHull, 0.0f);
  b2Polygon backCap = b2MakePolygon(&backCapHull, 0.0f);

  // Create shapes for caps using same shape definition
  b2CreatePolygonShape(bodyId, &shapeDef, &frontCap);
  b2CreatePolygonShape(bodyId, &shapeDef, &backCap);
}

b2ShapeId PhysicsSystem::createCircleShape(b2BodyId bodyId, float radius,
  uint16_t categoryBits, uint16_t maskBits,
  CollisionType collisionType,
  float density, float friction) {

  if (!b2Body_IsValid(bodyId)) return b2_nullShapeId;

  b2ShapeDef shapeDef = b2DefaultShapeDef();

  // Configure physics based on collision type
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

  default:  // DEFAULT type
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = 0.1f;
    break;
  }

  // Set collision filtering
  shapeDef.filter.categoryBits = categoryBits;
  shapeDef.filter.maskBits = maskBits;

  // Create circle shape
  b2Circle circle;
  circle.radius = radius;
  circle.center = b2Vec2_zero;

  // Create and return the shape
  b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
  if (!b2Shape_IsValid(shapeId)) {
    std::cerr << "Failed to create circle shape.\n";
  }
  return shapeId;
}

void PhysicsSystem::findOverlappingBodies(b2BodyId carBody, Car* car) {
  if (!car || !b2Body_IsValid(carBody)) return;

  b2Vec2 carPos = b2Body_GetPosition(carBody);
  float carRadius = 7.0f;
  float detectionRadius = carRadius + 10.0f;

  for (b2BodyId otherId : m_dynamicBodies) {
    if (!b2Body_IsValid(otherId) || otherId.index1 == carBody.index1) continue;

    void* userData = b2Body_GetUserData(otherId);
    if (!userData) continue;

    PlaceableObject* obj = static_cast<PlaceableObject*>(userData);
    if (!obj) continue;

    b2Vec2 otherPos = b2Body_GetPosition(otherId);
    b2Vec2 diff = { otherPos.x - carPos.x, otherPos.y - carPos.y };
    float distSq = diff.x * diff.x + diff.y * diff.y;

    if (distSq < detectionRadius * detectionRadius && (obj->isBooster() || obj->isXPPickup())) {
      car->onSensorEnter(otherId);
    }
  }
}

bool PhysicsSystem::checkIsPowerup(b2BodyId bodyId) {
  if (!b2Body_IsValid(bodyId)) return false;

  void* userData = b2Body_GetUserData(bodyId);
  if (!userData) return false;

  PlaceableObject* obj = static_cast<PlaceableObject*>(userData);
  return obj && (obj->isBooster() || obj->isXPPickup());
}

bool PhysicsSystem::onPreSolve(b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* context) {
  return handleContact(shapeIdA, shapeIdB);
}

bool PhysicsSystem::handleContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB) {
  if (!b2Shape_IsValid(shapeIdA) || !b2Shape_IsValid(shapeIdB)) {
    std::cout << "Invalid shapes in contact\n";
    return true;
  }

  // Get filter data directly from shapes
  b2Filter filterA = b2Shape_GetFilter(shapeIdA);
  b2Filter filterB = b2Shape_GetFilter(shapeIdB);

  std::cout << "\nHandling contact:\n";
  std::cout << "Shape A category: 0x" << std::hex << filterA.categoryBits << std::dec << "\n";
  std::cout << "Shape B category: 0x" << std::hex << filterB.categoryBits << std::dec << "\n";

  // Get the bodies that own these shapes
  b2BodyId bodyIdA = b2Shape_GetBody(shapeIdA);
  b2BodyId bodyIdB = b2Shape_GetBody(shapeIdB);

  // Get user data from bodies
  void* userDataA = b2Body_GetUserData(bodyIdA);
  void* userDataB = b2Body_GetUserData(bodyIdB);

  // Try to identify the colliding objects
  if (userDataA) {
    PlaceableObject* objA = static_cast<PlaceableObject*>(userDataA);
    std::cout << "Object A: " << objA->getDisplayName() << "\n";
  }
  if (userDataB) {
    PlaceableObject* objB = static_cast<PlaceableObject*>(userDataB);
    std::cout << "Object B: " << objB->getDisplayName() << "\n";
  }

  Car* carA = static_cast<Car*>(userDataA);
  Car* carB = static_cast<Car*>(userDataB);

  if (carA && filterB.categoryBits == CATEGORY_POWERUP) {
    std::cout << "Car A colliding with powerup\n";
    carA->onSensorEnter(bodyIdB);
  }
  else if (carB && filterA.categoryBits == CATEGORY_POWERUP) {
    std::cout << "Car B colliding with powerup\n";
    carB->onSensorEnter(bodyIdA);
  }

  return true;
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

void PhysicsSystem::synchronizeTransforms() {
  if (!b2World_IsValid(m_worldId)) return;

  // Clean up any invalid bodies
  auto newEnd = std::remove_if(m_dynamicBodies.begin(), m_dynamicBodies.end(),
    [](b2BodyId bodyId) { return !b2Body_IsValid(bodyId); });
  m_dynamicBodies.erase(newEnd, m_dynamicBodies.end());

  // Process each dynamic body
  for (b2BodyId bodyId : m_dynamicBodies) {
    b2Vec2 position = b2Body_GetPosition(bodyId);
    const float WRAP_THRESHOLD = 1000000.0f;

    bool needsWrap = false;
    if (std::abs(position.x) > WRAP_THRESHOLD) {
      position.x = std::copysign(10000.0f, position.x);
      needsWrap = true;
    }
    if (std::abs(position.y) > WRAP_THRESHOLD) {
      position.y = std::copysign(10000.0f, position.y);
      needsWrap = true;
    }

    if (needsWrap) {
      b2Rot rotation = b2Body_GetRotation(bodyId);
      b2Vec2 linearVel = b2Body_GetLinearVelocity(bodyId);
      float angularVel = b2Body_GetAngularVelocity(bodyId);

      b2Body_SetTransform(bodyId, position, rotation);
      b2Body_SetLinearVelocity(bodyId, linearVel);
      b2Body_SetAngularVelocity(bodyId, angularVel);
    }
  }
}
