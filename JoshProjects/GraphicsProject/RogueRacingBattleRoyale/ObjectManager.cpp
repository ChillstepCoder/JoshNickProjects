// ObjectManager.cpp

#include "ObjectManager.h"
#include "Car.h"
#include <set>
#include <iostream>
#include <algorithm>

ObjectManager::ObjectManager(SplineTrack* track, PhysicsSystem* physicsSystem)
  : m_track(track)
  , m_physicsSystem(physicsSystem)
  , m_selectedObject(nullptr) {
}

void ObjectManager::addObject(size_t templateIndex, const glm::vec2& position) {
  if (templateIndex >= m_objectTemplates.size()) {
    if (DEBUG_OUTPUT) {
      std::cout << "Invalid template index: " << templateIndex << "\n";
    }
    return;
  }
  if (DEBUG_OUTPUT) {
    std::cout << "\nAdding new object from template " << templateIndex << "\n";
    std::cout << "Template RTTI type: " << typeid(*m_objectTemplates[templateIndex]).name() << std::endl;
  }

  auto newObject = m_objectTemplates[templateIndex]->clone();
  if (DEBUG_OUTPUT) {
    std::cout << "New object display name: " << newObject->getDisplayName() << "\n";
    std::cout << "Is XP pickup: " << (newObject->isXPPickup() ? "yes" : "no") << "\n";
    std::cout << "Collision type: " << static_cast<int>(newObject->getCollisionType()) << "\n";
    std::cout << "Cloned RTTI type: " << typeid(*newObject).name() << std::endl;
  }

  newObject->setPosition(position);

  // Handle track alignment for objects that need it
  if (newObject->shouldAutoAlignToTrack() && m_track) {
    auto splinePoints = m_track->getSplinePoints(200);
    float minDist = std::numeric_limits<float>::max();
    size_t closestIdx = 0;

    // Find closest spline point
    for (size_t i = 0; i < splinePoints.size(); i++) {
      float dist = glm::distance(position, splinePoints[i].position);
      if (dist < minDist) {
        minDist = dist;
        closestIdx = i;
      }
    }

    // Calculate track direction at closest point
    size_t nextIdx = (closestIdx + 1) % splinePoints.size();
    glm::vec2 direction = glm::normalize(
      splinePoints[nextIdx].position - splinePoints[closestIdx].position);

    // Calculate angle from direction vector
    float angle = atan2(direction.y, direction.x);
    newObject->setRotation(angle);
  }

  createPhysicsForObject(newObject.get());
  m_placedObjects.push_back(std::move(newObject));
  updateGrid();
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

  if (DEBUG_OUTPUT) {
    std::cout << "\n=== Creating Physics for Object ===\n";
    std::cout << "Object type: " << obj->getDisplayName() << std::endl;
    std::cout << "Collision type: " << static_cast<int>(obj->getCollisionType()) << std::endl;
    std::cout << "Object address: " << obj << std::endl;
    std::cout << "Dynamic RTTI type: " << typeid(*obj).name() << std::endl;
  }

  b2BodyId bodyId;
  if (obj->getCollisionType() == CollisionType::POWERUP ||
    obj->getCollisionType() == CollisionType::PUSHABLE) {
    bodyId = m_physicsSystem->createDynamicBody(obj->getPosition().x, obj->getPosition().y);
  }
  else {
    bodyId = m_physicsSystem->createStaticBody(obj->getPosition().x, obj->getPosition().y);
  }

  if (!b2Body_IsValid(bodyId)) return;

  // Set transform and user data
  b2Rot rotation = b2MakeRot(obj->getRotation());
  b2Body_SetTransform(bodyId, b2Vec2{ obj->getPosition().x, obj->getPosition().y }, rotation);
  b2Body_SetUserData(bodyId, static_cast<void*>(obj));

  // Debug output before shape creation
  if (DEBUG_OUTPUT) {
    std::cout << "About to create collision shape for: " << obj->getDisplayName()
      << " CollisionType: " << static_cast<int>(obj->getCollisionType()) << std::endl;
  }

  // Let the object create its own collision shape
  if (DEBUG_OUTPUT) {
    std::cout << "Calling object's createCollisionShape..." << std::endl;
  }
    obj->createCollisionShape(bodyId, m_physicsSystem);

  // Store the physics body ID
  if (DEBUG_OUTPUT) {
    std::cout << "Setting Physics Body\n";
  }
  obj->setPhysicsBody(bodyId);
  if (DEBUG_OUTPUT) {
    std::cout << "=== End Physics Creation ===\n";
  }
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
  // Update object positions and timers
  for (auto& obj : m_placedObjects) {
    if (!obj) continue;

    if (m_physicsSystem && b2Body_IsValid(obj->getPhysicsBody())) {
      b2Vec2 pos = b2Body_GetPosition(obj->getPhysicsBody());
      float angle = b2Rot_GetAngle(b2Body_GetRotation(obj->getPhysicsBody()));
      obj->setPosition(glm::vec2(pos.x, pos.y));
      obj->setRotation(angle);

      // Update physics properties for pushable objects
      if (obj->getCollisionType() == CollisionType::PUSHABLE) {
        b2Body_SetLinearDamping(obj->getPhysicsBody(), 4.0f);
        b2Body_SetAngularDamping(obj->getPhysicsBody(), 4.0f);
      }
    }

    if (obj->isXPPickup()) {
      obj->updateRespawnTimer(1.0f / 60.0f);
    }
  }

  // Update grid
  m_grid.clear();
  for (const auto& obj : m_placedObjects) {
    if (obj) {
      int64_t cell = getGridCell(obj->getPosition());
      m_grid[cell].push_back(obj.get());
    }
  }
}


void ObjectManager::createDefaultTemplates() {
  std::cout << "\nCreating default templates...\n";
  m_objectTemplates.emplace_back(std::make_unique<PotholeObject>());
  m_objectTemplates.emplace_back(std::make_unique<TreeObject>());
  m_objectTemplates.emplace_back(std::make_unique<TrafficConeObject>());
  m_objectTemplates.emplace_back(std::make_unique<BoosterObject>());
  m_objectTemplates.emplace_back(std::make_unique<XPPickupObject>());
}



void ObjectManager::addTemplate(std::unique_ptr<PlaceableObject> templ) {
  m_objectTemplates.push_back(std::move(templ));
}

void ObjectManager::setCars(const std::vector<std::unique_ptr<Car>>& cars) {
  m_cars.clear();

  if (DEBUG_OUTPUT) {
    std::cout << "Setting cars in ObjectManager (" << this << ")" << std::endl;
    std::cout << "Number of cars being added: " << cars.size() << std::endl;
  }

  for (const auto& car : cars) {
    m_cars.push_back(car.get());
    if (DEBUG_OUTPUT) {
      std::cout << "Added car at position ("
        << car->getDebugInfo().position.x << ", "
        << car->getDebugInfo().position.y << ")" << std::endl;
    }
  }

  if (DEBUG_OUTPUT) {
    std::cout << "Final car count in ObjectManager: " << m_cars.size() << std::endl;
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

int64_t ObjectManager::getGridCell(const glm::vec2& pos) const {
  int x = static_cast<int>(std::floor(pos.x / CELL_SIZE));
  int y = static_cast<int>(std::floor(pos.y / CELL_SIZE));
  return (static_cast<int64_t>(x) << 32) | static_cast<int64_t>(y);
}

void ObjectManager::updateGrid() {
  // Clear vectors
  for (auto& pair : m_grid) {
    pair.second.clear();
  }

  // Re-add all objects to their current cells
  for (const auto& obj : m_placedObjects) {
    if (obj) {
      int64_t cell = getGridCell(obj->getPosition());
      m_grid[cell].push_back(obj.get());
    }
  }
}

std::vector<const PlaceableObject*> ObjectManager::getNearbyObjects(const glm::vec2& pos, float radius) {
  std::vector<const PlaceableObject*> nearby;
  nearby.reserve(16);

  // Calculate grid cells to check
  int cellRadius = static_cast<int>(std::ceil(radius / CELL_SIZE));
  int centerX = static_cast<int>(std::floor(pos.x / CELL_SIZE));
  int centerY = static_cast<int>(std::floor(pos.y / CELL_SIZE));

  std::set<const PlaceableObject*> uniqueObjects;

  // Iterate through cells
  for (int y = centerY - cellRadius; y <= centerY + cellRadius; y++) {
    for (int x = centerX - cellRadius; x <= centerX + cellRadius; x++) {
      int64_t cell = (static_cast<int64_t>(x) << 32) | static_cast<int64_t>(y);
      auto it = m_grid.find(cell);
      if (it != m_grid.end()) {
        // Check each object in the cell
        for (auto* obj : it->second) {
          if (obj && uniqueObjects.insert(obj).second) {
            nearby.push_back(obj);
          }
        }
      }
    }
  }

  return nearby;
}
