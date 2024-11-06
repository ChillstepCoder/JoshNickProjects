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
    if (!obj || !m_track) {
      std::cout << "Invalid placement check - null object or track\n";
      return false;
    }

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
    float offroadEdge = nearestPoint.roadWidth +
      std::max(nearestPoint.offroadWidth.x, nearestPoint.offroadWidth.y);

    bool isValid = false;
    switch (obj->getZone()) {
    case PlacementZone::Road:
      isValid = roadDist <= nearestPoint.roadWidth;
      std::cout << "Road placement check - distance: " << roadDist << ", limit: " << nearestPoint.roadWidth << "\n";
      break;
    case PlacementZone::Offroad:
      isValid = roadDist > nearestPoint.roadWidth && roadDist <= offroadEdge;
      std::cout << "Offroad placement check - distance: " << roadDist << ", range: " << nearestPoint.roadWidth << " to " << offroadEdge << "\n";
      break;
    case PlacementZone::Grass:
      isValid = roadDist > offroadEdge;
      std::cout << "Grass placement check - distance: " << roadDist << ", minimum: " << offroadEdge << "\n";
      break;
    case PlacementZone::Anywhere:
      isValid = true;
      break;
    }

    std::cout << "Placement valid: " << (isValid ? "yes" : "no") << "\n";
    return isValid;
  }

  const std::vector<std::unique_ptr<PlaceableObject>>& getPlacedObjects() const {
    return m_placedObjects;
  }

  const std::vector<std::unique_ptr<PlaceableObject>>& getObjectTemplates() const {
    return m_objectTemplates;
  }

private:
  SplineTrack* m_track;
  std::vector<std::unique_ptr<PlaceableObject>> m_objectTemplates;
  std::vector<std::unique_ptr<PlaceableObject>> m_placedObjects;
};
