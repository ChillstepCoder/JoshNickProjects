// ObjectManager.h

#pragma once
#include <vector>
#include <memory>
#include "PlaceableObject.h"
#include "SplineTrack.h"
#include "PhysicsSystem.h"
#include "Car.h"

class ObjectManager {
public:
  ObjectManager(SplineTrack* track, PhysicsSystem* physicsSystem)
    : m_track(track)
    , m_physicsSystem(physicsSystem) {
  }

  void addObject(size_t templateIndex, const glm::vec2& position) {
    if (templateIndex >= m_objectTemplates.size()) {
      std::cout << "Invalid template index: " << templateIndex << "\n";
      return;
    }

    // Create new object as a copy of the template
    auto newObject = std::make_unique<PlaceableObject>(*m_objectTemplates[templateIndex]);
    newObject->setPosition(position);

    // Create physics for the object
    createPhysicsForObject(newObject.get());

    m_placedObjects.push_back(std::move(newObject));
  }

  void removeSelectedObject() {
    auto it = std::find_if(m_placedObjects.begin(), m_placedObjects.end(),
      [](const auto& obj) { return obj->isSelected(); });
    if (it != m_placedObjects.end()) {
      // Physics cleanup is handled by PlaceableObject destructor
      m_placedObjects.erase(it);
    }
  }

  PlaceableObject* getObjectAtPosition(const glm::vec2& position) {
    for (auto& obj : m_placedObjects) {
      glm::vec4 bounds = obj->getBounds();
      if (position.x >= bounds.x && position.x <= bounds.x + bounds.z &&
        position.y >= bounds.y && position.y <= bounds.y + bounds.w) {
        return obj.get();
      }
    }
    return nullptr;
  }

  bool isValidPlacement(const PlaceableObject* obj, const glm::vec2& position) const {
    if (!obj || !m_track) return false;

    auto splinePoints = m_track->getSplinePoints(100);
    float minDist = std::numeric_limits<float>::max();
    SplineTrack::SplinePointInfo nearestPoint;

    // Find nearest spline point
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

    // Special case for trees - allow in grass and offroad
    if (obj->getDisplayName().find("tree") != std::string::npos) {
      return roadDist > roadWidth;  // Trees can be anywhere except the road
    }

    // Normal placement rules for other objects
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

  void createPhysicsForObject(PlaceableObject* obj) {
    if (!m_physicsSystem || !obj) return;

    // Create physics body
    b2BodyId bodyId;
    if (obj->getCollisionType() == PhysicsSystem::CollisionType::PUSHABLE) {
      bodyId = m_physicsSystem->createDynamicBody(obj->getPosition().x, obj->getPosition().y);
    }
    else {
      bodyId = m_physicsSystem->createStaticBody(obj->getPosition().x, obj->getPosition().y);
    }

    // Set up collision filtering based on type
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

    default:  // DEFAULT type (trees, etc)
      categoryBits = PhysicsSystem::CATEGORY_SOLID;
      maskBits = PhysicsSystem::CATEGORY_CAR |
        PhysicsSystem::CATEGORY_PUSHABLE;
      break;
    }

    // Create circular collision shape
    float radius;
    if (obj->getDisplayName().find("tree") != std::string::npos) {
      radius = 10.0f;  // Larger collision for trees
    }
    else if (obj->getDisplayName().find("cone") != std::string::npos) {
      radius = 5.0f;   // Small collision for cones
    }
    else {
      radius = 7.5f;   // Default size for other objects
    }

    b2ShapeId shapeId = m_physicsSystem->createCircleShape(bodyId, radius,
      categoryBits, maskBits, obj->getCollisionType());

    // For pushable objects, set appropriate mass and damping
    if (obj->getCollisionType() == PhysicsSystem::CollisionType::PUSHABLE) {
      b2MassData massData;
      massData.mass = 1.0f;  // Light weight
      massData.center = b2Vec2_zero;
      massData.rotationalInertia = 0.5f;
      b2Body_SetMassData(bodyId, massData);

      b2Body_SetLinearDamping(bodyId, 2.0f);  // More damping for smoother movement
      b2Body_SetAngularDamping(bodyId, 4.0f);
    }

    // Store the physics objects
    obj->setPhysicsBody(bodyId);
    obj->setCollisionShape(shapeId);
  }

  const std::vector<std::unique_ptr<PlaceableObject>>& getPlacedObjects() const {
    return m_placedObjects;
  }

  const std::vector<std::unique_ptr<PlaceableObject>>& getObjectTemplates() const {
    return m_objectTemplates;
  }
  void removeInvalidObjects(const std::vector<PlaceableObject*>& objectsToRemove) {
    size_t originalSize = m_placedObjects.size();

    // Remove objects that are no longer valid
    m_placedObjects.erase(
      std::remove_if(m_placedObjects.begin(), m_placedObjects.end(),
        [&](const std::unique_ptr<PlaceableObject>& obj) {
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

    // If the selected object was removed, clear the selection
    if (removedCount > 0) {
      for (const auto& obj : objectsToRemove) {
        if (obj == m_selectedObject) {
          m_selectedObject = nullptr;
          break;
        }
      }
    }
  }

  bool isSelected(const PlaceableObject* obj) const {
    return m_selectedObject == obj;
  }

  void update() {
    // Update visual positions of objects based on physics bodies
    for (auto& obj : m_placedObjects) {
      if (m_physicsSystem && b2Body_IsValid(obj->getPhysicsBody())) {
        // Get position from physics body
        b2Vec2 pos = b2Body_GetPosition(obj->getPhysicsBody());
        float angle = b2Rot_GetAngle(b2Body_GetRotation(obj->getPhysicsBody()));

        // Update visual position and rotation
        obj->setPosition(glm::vec2(pos.x, pos.y));
        obj->setRotation(angle);
      }
    }
  }

  void addTemplate(std::unique_ptr<PlaceableObject> templ) {
    m_objectTemplates.push_back(std::move(templ));
  }

  void createDefaultTemplates() {
    m_objectTemplates.emplace_back(
      std::make_unique<PlaceableObject>("Textures/pothole.png", PlacementZone::Road));
    m_objectTemplates.emplace_back(
      std::make_unique<PlaceableObject>("Textures/tree.png", PlacementZone::Grass));
    m_objectTemplates.emplace_back(
      std::make_unique<PlaceableObject>("Textures/traffic_cone.png", PlacementZone::Anywhere));
  }

  // Setters
  void setCars(const std::vector<std::unique_ptr<Car>>& cars) {
    m_cars.clear();
    for (const auto& car : cars) {
      m_cars.push_back(car.get());
    }
  }

  void clearCars() {
    m_cars.clear();
  }

private:
  SplineTrack* m_track;
  std::vector<std::unique_ptr<PlaceableObject>> m_objectTemplates;
  std::vector<std::unique_ptr<PlaceableObject>> m_placedObjects;
  PlaceableObject* m_selectedObject = nullptr;
  PhysicsSystem* m_physicsSystem = nullptr;
  std::vector<Car*> m_cars;
};
