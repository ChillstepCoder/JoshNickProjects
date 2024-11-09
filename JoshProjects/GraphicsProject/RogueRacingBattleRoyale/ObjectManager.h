// ObjectManager.h

#pragma once
#include <vector>
#include <memory>
#include "PlaceableObject.h"
#include "SplineTrack.h"

class ObjectManager {
public:
  ObjectManager(SplineTrack* track) : m_track(track) {
    // Initialize with default objects
    m_objectTemplates.emplace_back(
      std::make_unique<PlaceableObject>("Textures/pothole.png", PlacementZone::Road));
    m_objectTemplates.emplace_back(
      std::make_unique<PlaceableObject>("Textures/tree.png", PlacementZone::Grass));
    m_objectTemplates.emplace_back(
      std::make_unique<PlaceableObject>("Textures/traffic_cone.png", PlacementZone::Anywhere));
  }

  void addObject(size_t templateIndex, const glm::vec2& position) {
    if (templateIndex >= m_objectTemplates.size()) {
      std::cout << "Invalid template index: " << templateIndex << "\n";
      return;
    }

    // Create new object as a copy of the template
    auto newObject = std::make_unique<PlaceableObject>(*m_objectTemplates[templateIndex]);
    newObject->setPosition(position);

    // Debug output
    std::cout << "Adding object at position: " << position.x << ", " << position.y << "\n";
    std::cout << "Object type: " << newObject->getDisplayName() << "\n";
    std::cout << "Total placed objects before: " << m_placedObjects.size() << "\n";

    m_placedObjects.push_back(std::move(newObject));

    std::cout << "Total placed objects after: " << m_placedObjects.size() << "\n";
  }

  void removeSelectedObject() {
    auto it = std::find_if(m_placedObjects.begin(), m_placedObjects.end(),
      [](const auto& obj) { return obj->isSelected(); });
    if (it != m_placedObjects.end()) {
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

    // Calculate distances for different zones
    float roadDist = minDist;
    float roadWidth = nearestPoint.roadWidth;
    float offroadWidth = std::max(nearestPoint.offroadWidth.x, nearestPoint.offroadWidth.y);
    float offroadEdge = roadWidth + offroadWidth;

    // Check placement validity based on zone
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

private:
  SplineTrack* m_track;
  std::vector<std::unique_ptr<PlaceableObject>> m_objectTemplates;
  std::vector<std::unique_ptr<PlaceableObject>> m_placedObjects;
  PlaceableObject* m_selectedObject = nullptr;
};
