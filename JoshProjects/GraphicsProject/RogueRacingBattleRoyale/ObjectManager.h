// ObjectManager.h

#pragma once
#include <vector>
#include <memory>

#include "PlaceableObject.h"
#include "BoosterObject.h"
#include "XPPickupObject.h"
#include "PotholeObject.h"
#include "TreeObject.h"
#include "TrafficConeObject.h"

#include "SplineTrack.h"
#include "PhysicsSystem.h"
#include <unordered_map>

class Car;

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
  void updateGrid();

  // Car Management
  void addCar(Car* car) { m_cars.push_back(car); }
  void setCars(const std::vector<std::unique_ptr<Car>>& cars);
  void clearCars();

  // Getters
  std::vector<const PlaceableObject*> getNearbyObjects(const glm::vec2& pos, float radius);
  const std::vector<Car*>& getCars() const { return m_cars; }
  int64_t getGridCell(const glm::vec2& pos) const;
  const std::vector<std::unique_ptr<PlaceableObject>>& getPlacedObjects() const;
  const std::vector<std::unique_ptr<PlaceableObject>>& getObjectTemplates() const;
  SplineTrack* getTrack() const { return m_track; }

  friend class AIDriver;

private:
  static constexpr bool DEBUG_OUTPUT = false;

  SplineTrack* m_track;
  PhysicsSystem* m_physicsSystem;
  std::vector<std::unique_ptr<PlaceableObject>> m_objectTemplates;
  std::vector<std::unique_ptr<PlaceableObject>> m_placedObjects;
  PlaceableObject* m_selectedObject;
  std::vector<Car*> m_cars;

  static constexpr float CELL_SIZE = 100.0f;
  std::unordered_map<int64_t, std::vector<PlaceableObject*>> m_grid;
};
