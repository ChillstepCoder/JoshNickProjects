// ObjectManager.cpp

#include "ObjectManager.h"
#include <iostream>
#include <algorithm>

ObjectManager::ObjectManager(SplineTrack* track, PhysicsSystem* physicsSystem)
  : m_track(track)
  , m_physicsSystem(physicsSystem)
  , m_selectedObject(nullptr) {
}

void ObjectManager::addObject(size_t templateIndex, const glm::vec2& position) {
  if (templateIndex >= m_objectTemplates.size()) {
    std::cout << "Invalid template index: " << templateIndex << "\n";
    return;
  }

  auto newObject = std::make_unique<PlaceableObject>(*m_objectTemplates[templateIndex]);
  newObject->setPosition(position);
  createPhysicsForObject(newObject.get());
  m_placedObjects.push_back(std::move(newObject));
}

void ObjectManager::removeSelectedObject() {
  auto it = std::find_if(m_placedObjects.begin(), m_placedObjects.end(),
    [](const auto& obj) { return obj->isSelected(); });
  if (it != m_placedObjects.end()) {
    m_placedObjects.erase(it);
  }
}

PlaceableObject* ObjectManager::getObjectAtPosition(const glm::vec2& position) {
  for (auto& obj : m_placedObjects) {
    glm::vec4 bounds = obj->getBounds();
    if (position.x >= bounds.x && position.x <= bounds.x + bounds.z &&
      position.y >= bounds.y && position.y <= bounds.y + bounds.w) {
      return obj.get();
    }
  }
  return nullptr;
}

bool ObjectManager::isValidPlacement(const PlaceableObject* obj, const glm::vec2& position) const {
  if (!obj || !m_track) return false;

  auto splinePoints = m_track->getSplinePoints(100);
  float minDist = std::numeric_limits<float>::max();
  SplineTrack::SplinePointInfo nearestPoint;

  for (const auto& point : splinePoints) {
    float dist = glm::distance(position, point.position);
    if (dist < minDist) {
      minDist = dist;
      nearestPoint = point;
    }
  }

  float roadDist = minDist;
  float roadWidth = nearestPoint.roadWidth;
  float offroadWidth = std::max(nearestPoint.offroadWidth.x, nearestPoint.offroadWidth.y);
  float offroadEdge = roadWidth + offroadWidth;

  if (obj->getDisplayName().find("tree") != std::string::npos) {
    return roadDist > roadWidth;
  }

  switch (obj->getZone()) {
  case PlacementZone::Road:
    return roadDist <= roadWidth;
  case PlacementZone::Offroad:
    return roadDist > roadWidth && roadDist <= offroadEdge;
  case PlacementZone::Grass:
    return roadDist > offroadEdge;
  case PlacementZone::Anywhere:
    return true;
  default:
    return false;
  }
}

void ObjectManager::createPhysicsForObject(PlaceableObject* obj) {
  if (!m_physicsSystem || !obj) return;

  b2BodyId bodyId;
  if (obj->getCollisionType() == PhysicsSystem::CollisionType::PUSHABLE) {
    bodyId = m_physicsSystem->createDynamicBody(obj->getPosition().x, obj->getPosition().y);
  }
  else {
    bodyId = m_physicsSystem->createStaticBody(obj->getPosition().x, obj->getPosition().y);
  }

  uint16_t categoryBits;
  uint16_t maskBits;

  switch (obj->getCollisionType()) {
  case PhysicsSystem::CollisionType::HAZARD:
    categoryBits = PhysicsSystem::CATEGORY_HAZARD;
    maskBits = PhysicsSystem::CATEGORY_CAR;
    break;

  case PhysicsSystem::CollisionType::PUSHABLE:
    categoryBits = PhysicsSystem::CATEGORY_PUSHABLE;
    maskBits = PhysicsSystem::CATEGORY_CAR |
      PhysicsSystem::CATEGORY_SOLID |
      PhysicsSystem::CATEGORY_PUSHABLE |
      PhysicsSystem::CATEGORY_BARRIER;
    break;

  default:
    categoryBits = PhysicsSystem::CATEGORY_SOLID;
    maskBits = PhysicsSystem::CATEGORY_CAR |
      PhysicsSystem::CATEGORY_PUSHABLE;
    break;
  }

  float radius = obj->getDisplayName().find("tree") != std::string::npos ? 10.0f :
    obj->getDisplayName().find("cone") != std::string::npos ? 5.0f : 7.5f;

  b2ShapeId shapeId = m_physicsSystem->createCircleShape(bodyId, radius,
    categoryBits, maskBits, obj->getCollisionType());

  if (obj->getCollisionType() == PhysicsSystem::CollisionType::PUSHABLE) {
    b2MassData massData;
    massData.mass = 1.0f;
    massData.center = b2Vec2_zero;
    massData.rotationalInertia = 0.5f;
    b2Body_SetMassData(bodyId, massData);
    b2Body_SetLinearDamping(bodyId, 2.0f);
    b2Body_SetAngularDamping(bodyId, 4.0f);
  }

  obj->setPhysicsBody(bodyId);
  obj->setCollisionShape(shapeId);
}

void ObjectManager::removeInvalidObjects(const std::vector<PlaceableObject*>& objectsToRemove) {
  size_t originalSize = m_placedObjects.size();

  m_placedObjects.erase(
    std::remove_if(m_placedObjects.begin(), m_placedObjects.end(),
      [&](const auto& obj) {
        bool shouldRemove = std::find(objectsToRemove.begin(),
          objectsToRemove.end(),
          obj.get()) != objectsToRemove.end();
        if (shouldRemove) {
          std::cout << "Removing invalid object at position ("
            << obj->getPosition().x << ", "
            << obj->getPosition().y << ")\n";
        }
        return shouldRemove;
      }
    ),
    m_placedObjects.end()
  );

  size_t removedCount = originalSize - m_placedObjects.size();
  std::cout << "Removed " << removedCount << " invalid objects\n";

  if (removedCount > 0) {
    for (const auto& obj : objectsToRemove) {
      if (obj == m_selectedObject) {
        m_selectedObject = nullptr;
        break;
      }
    }
  }
}

void ObjectManager::update() {
  for (auto& obj : m_placedObjects) {
    if (m_physicsSystem && b2Body_IsValid(obj->getPhysicsBody())) {
      b2Vec2 pos = b2Body_GetPosition(obj->getPhysicsBody());
      float angle = b2Rot_GetAngle(b2Body_GetRotation(obj->getPhysicsBody()));
      obj->setPosition(glm::vec2(pos.x, pos.y));
      obj->setRotation(angle);
    }
  }
}

void ObjectManager::createDefaultTemplates() {
  m_objectTemplates.emplace_back(
    std::make_unique<PlaceableObject>("Textures/pothole.png", PlacementZone::Road));
  m_objectTemplates.emplace_back(
    std::make_unique<PlaceableObject>("Textures/tree.png", PlacementZone::Grass));
  m_objectTemplates.emplace_back(
    std::make_unique<PlaceableObject>("Textures/traffic_cone.png", PlacementZone::Anywhere));
}

void ObjectManager::addTemplate(std::unique_ptr<PlaceableObject> templ) {
  m_objectTemplates.push_back(std::move(templ));
}

void ObjectManager::setCars(const std::vector<std::unique_ptr<Car>>& cars) {
  m_cars.clear();
  for (const auto& car : cars) {
    m_cars.push_back(car.get());
  }
}

void ObjectManager::clearCars() {
  m_cars.clear();
}

bool ObjectManager::isSelected(const PlaceableObject* obj) const {
  return m_selectedObject == obj;
}

const std::vector<std::unique_ptr<PlaceableObject>>& ObjectManager::getPlacedObjects() const {
  return m_placedObjects;
}

const std::vector<std::unique_ptr<PlaceableObject>>& ObjectManager::getObjectTemplates() const {
  return m_objectTemplates;
}
