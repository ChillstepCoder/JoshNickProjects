//PhysicsSystem.cpp

#include "PhysicsSystem.h"
#include "Car.h"
#include "PlaceableObject.h"
#include "XPPickupObject.h"
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

  if (DEBUG_OUTPUT) {
    std::cout << "Created physics world with ID: " << (unsigned)m_worldId.index1 << std::endl;
  }
}

void PhysicsSystem::update(float timeStep) {
  if (!b2World_IsValid(m_worldId)) {
    std::cerr << "Invalid world ID in PhysicsSystem::update!" << std::endl;
    return;
  }

  if (DEBUG_OUTPUT) {
    std::cout << "Updating physics world ID: " << (unsigned)m_worldId.index1 << std::endl;
  }

  // Physics step
  const float fixedTimeStep = 1.0f / 60.0f;
  const int subStepCount = static_cast<int>(std::ceil(timeStep / fixedTimeStep));

  try {
    for (int i = 0; i < subStepCount; i++) {
      if (!b2World_IsValid(m_worldId)) {
        std::cerr << "World became invalid during step!" << std::endl;
        return;
      }
      b2World_Step(m_worldId, fixedTimeStep, 1);
    }

    handleSensorEvents();
    synchronizeTransforms();
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in physics update: " << e.what() << std::endl;
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
      if (DEBUG_OUTPUT) {
        std::cout << "Adding new dynamic body " << bodyId.index1 << " to tracking list" << std::endl;
      }
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
    shapeDef.enableSensorEvents = true;
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

  if (!b2Body_IsValid(bodyId)) return b2_nullShapeId;

  b2ShapeDef shapeDef = b2DefaultShapeDef();

  // Get the object from body user data
  void* userData = b2Body_GetUserData(bodyId);
  PlaceableObject* object = static_cast<PlaceableObject*>(userData);

  // Configure shape based on object type and collision type
  if (object && object->isSensor()) {
    // Configure as sensor
    shapeDef.isSensor = true;
    shapeDef.enableSensorEvents = true;
    shapeDef.density = 0.0f;
    shapeDef.friction = 0.0f;
    shapeDef.restitution = 0.0f;
  }
  else {
    // Configure as normal physics object
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = 0.1f;
  }

  shapeDef.filter.categoryBits = categoryBits;
  shapeDef.filter.maskBits = maskBits;

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
  if (!obj) return false;

  ObjectType type = obj->getObjectType();
  return type == ObjectType::Booster || type == ObjectType::XPPickup;
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

bool PhysicsSystem::isValidUserData(void* userData) const {
  // Check if pointer is null or suspiciously small/invalid
  return userData != nullptr && reinterpret_cast<uintptr_t>(userData) > 1000;
}

bool PhysicsSystem::isCarBody(b2BodyId bodyId) const {
  if (!b2Body_IsValid(bodyId)) return false;

  void* userData = nullptr;
  userData = b2Body_GetUserData(bodyId);


  return isValidUserData(userData) &&
    static_cast<Car*>(userData) != nullptr;
}

bool PhysicsSystem::isObjectBody(b2BodyId bodyId) const {
  if (!b2Body_IsValid(bodyId)) return false;

  void* userData = nullptr;

  userData = b2Body_GetUserData(bodyId);

  if (!isValidUserData(userData)) {
    return false;
  }

  // Check if it has the correct type
  PlaceableObject* obj = static_cast<PlaceableObject*>(userData);
  return obj->getObjectType() != ObjectType::Default;
  
}
void PhysicsSystem::handleSensorEvents() {
  if (!b2World_IsValid(m_worldId)) return;

  b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_worldId);

  // Handle begin touch events
  for (int i = 0; i < sensorEvents.beginCount; ++i) {
    b2SensorBeginTouchEvent* beginTouch = sensorEvents.beginEvents + i;

    b2BodyId sensorBody = b2Shape_GetBody(beginTouch->sensorShapeId);
    b2BodyId visitorBody = b2Shape_GetBody(beginTouch->visitorShapeId);

    // Validate bodies exist and are the right types
    if (!isObjectBody(sensorBody) || !isCarBody(visitorBody)) {
      continue;
    }

    void* sensorData = b2Body_GetUserData(sensorBody);
    void* visitorData = b2Body_GetUserData(visitorBody);

    PlaceableObject* object = static_cast<PlaceableObject*>(sensorData);
    Car* car = static_cast<Car*>(visitorData);

    if (!object || !car) {
      continue;
    }

    // Check if it's an XP pickup and if it's active
    if (object->getObjectType() == ObjectType::XPPickup) {
      const auto& xpProps = object->getXPProperties();
      if (!xpProps.isActive) {
        continue;
      }
    }

    object->onCarCollision(car);
  }

  // Handle end touch events - same validation
  for (int i = 0; i < sensorEvents.endCount; ++i) {
    b2SensorEndTouchEvent* endTouch = sensorEvents.endEvents + i;

    b2BodyId sensorBody = b2Shape_GetBody(endTouch->sensorShapeId);
    b2BodyId visitorBody = b2Shape_GetBody(endTouch->visitorShapeId);

    if (!b2Shape_IsSensor(endTouch->sensorShapeId)) {
      // Sometimes box2d fucks up and inverts our sensor and visitor
      // TODO: We may not want to continue here, and actually just swap the bodies like
      // std::swap(SensorBody, visitorBody); so we can still process the end touch.
      continue;
    }

    void* sensorData = b2Body_GetUserData(sensorBody);
    void* visitorData = b2Body_GetUserData(visitorBody);

    PlaceableObject* object = static_cast<PlaceableObject*>(sensorData);
    Car* car = static_cast<Car*>(visitorData);

    if (!object || !car) {
      continue;
    }

    // Skip end collision events for XP pickups
    if (object->getObjectType() == ObjectType::XPPickup) {
      continue;
    }

    object->onEndCollision(car);
  }
}

void* PhysicsSystem::tryGetUserData(b2BodyId bodyId) {
  if (!b2Body_IsValid(bodyId)) {
    std::cout << "Body " << bodyId.index1 << " is invalid" << std::endl;
    return nullptr;
  }

    void* userData = b2Body_GetUserData(bodyId);
    if (!userData) {
      std::cout << "No user data for body " << bodyId.index1 << std::endl;
    }
    return userData;
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
