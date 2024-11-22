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
  // Constructor/Destructor
  ObjectManager(SplineTrack* track, PhysicsSystem* physicsSystem);
  ~ObjectManager() = default;

  // Object Creation and Management
  void addObject(size_t templateIndex, const glm::vec2& position);
  void addTemplate(std::unique_ptr<PlaceableObject> templ);
  void createDefaultTemplates();
  void removeSelectedObject();
  void removeInvalidObjects(const std::vector<PlaceableObject*>& objectsToRemove);

  // Validation and Queries
  bool isValidPlacement(const PlaceableObject* obj, const glm::vec2& position) const;
  PlaceableObject* getObjectAtPosition(const glm::vec2& position);
  bool isSelected(const PlaceableObject* obj) const;

  // Physics and Updates
  void createPhysicsForObject(PlaceableObject* obj);
  void update();

  // Car Management
  void setCars(const std::vector<std::unique_ptr<Car>>& cars);
  void clearCars();

  // Getters
  const std::vector<std::unique_ptr<PlaceableObject>>& getPlacedObjects() const;
  const std::vector<std::unique_ptr<PlaceableObject>>& getObjectTemplates() const;

private:
  SplineTrack* m_track;
  PhysicsSystem* m_physicsSystem;
  std::vector<std::unique_ptr<PlaceableObject>> m_objectTemplates;
  std::vector<std::unique_ptr<PlaceableObject>> m_placedObjects;
  PlaceableObject* m_selectedObject;
  std::vector<Car*> m_cars;
};
