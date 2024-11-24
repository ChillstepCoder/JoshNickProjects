// LevelEditorScreen.h

#pragma once
#include <JAGEngine/IGameScreen.h>
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/Camera2D.h>
#include <JAGEngine/ResourceManager.h>
#include <JAGEngine/GLTexture.h>
#include "ObjectManager.h"
#include "RoadMeshGenerator.h"
#include "LevelRenderer.h"
#include "Car.h"
#include "PhysicsSystem.h"
#include "InputState.h"
#include "DebugDraw.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <numeric>
#include <algorithm>
#include <ImGui/imgui.h>

// Forward declarations
class SplineTrack;
class TrackNode;

class LevelEditorScreen : public JAGEngine::IGameScreen {
public:
  // Public types
  enum class RoadViewMode {
    Spline,      // Shows spline points and control nodes
    Wireframe,   // Shows triangle mesh wireframe
    Shaded       // Shows fully shaded road
  };

  struct CarTrackingInfo {
    glm::vec2 closestSplinePoint;
    float distanceAlongSpline;
    int racePosition;
    bool showDebugPoint;
  };

  // Constructor/Destructor
  LevelEditorScreen();
  ~LevelEditorScreen();

  // IGameScreen interface implementation
  virtual void build() override;
  virtual void destroy() override;
  virtual void onEntry() override;
  virtual void onExit() override;
  virtual void update() override;
  virtual void draw() override;
  virtual int getNextScreenIndex() const override;
  virtual int getPreviousScreenIndex() const override;

private:
  // Rendering components
  JAGEngine::SpriteBatch m_spriteBatch;
  glm::mat4 m_projectionMatrix;
  std::unique_ptr<LevelRenderer> m_levelRenderer;

  // Track state
  std::unique_ptr<SplineTrack> m_track;
  TrackNode* m_selectedNode = nullptr;
  TrackNode* m_draggedNode = nullptr;
  bool m_isDragging = false;
  glm::vec2 m_lastMousePos;

  // Camera properties
  JAGEngine::Camera2D m_camera;
  float m_cameraSpeed = 150.0f;
  float m_zoomFactor = 1.02f;
  float m_minZoom = 0.1f;
  float m_maxZoom = 5.0f;
  glm::vec2 m_editorCameraPos = glm::vec2(0.0f);
  float m_editorCameraScale = 1.0f;
  float m_testCameraScale = 0.5f;
  bool m_canMoveCamera = true;

  // Constants
  static constexpr int MIN_LOD = 4;
  static constexpr int MAX_LOD = 50;
  static constexpr float MESSAGE_DURATION = 3.0f;

  // Node editing state
  bool m_addNodeMode = false;
  bool m_deleteObjectMode = false;
  glm::vec2 m_previewNodePosition = glm::vec2(0.0f);
  bool m_showPreviewNode = false;

  // Appearance settings
  glm::vec3 m_grassColor = glm::vec3(0.2f, 0.5f, 0.1f);
  glm::vec3 m_offroadColor = glm::vec3(0.45f, 0.32f, 0.15f);
  float m_grassNoiseScale = 100.0f;
  float m_grassNoiseIntensity = 1.0f;
  glm::vec3 m_barrierPrimaryColor = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 m_barrierSecondaryColor = glm::vec3(0.0f, 0.0f, 0.0f);
  float m_barrierPatternScale = 5.0f;

  // View settings
  RoadViewMode m_roadViewMode = RoadViewMode::Shaded;
  bool m_showSplinePoints = true;
  bool m_showStartPositions = true;
  int m_roadLOD = 50;
  bool m_autoUpdateMesh = true;

  // Object management
  std::unique_ptr<ObjectManager> m_objectManager;
  int m_selectedTemplateIndex = -1;
  bool m_objectPlacementMode = false;
  PlaceableObject* m_selectedObject = nullptr;
  bool m_isDraggingObject = false;
  glm::vec2 m_lastDragPos;

  // Test mode state
  bool m_testMode = false;
  std::vector<std::unique_ptr<Car>> m_testCars;
  std::unique_ptr<PhysicsSystem> m_physicsSystem;
  GLuint m_carTexture;
  std::unique_ptr<DebugDraw> m_debugDraw;
  bool m_showDebugDraw = false;
  std::vector<CarTrackingInfo> m_carTrackingInfo;
  float m_lookAheadDistance = 100.0f;
  float m_minCarScreenDistance = 100.0f;

  // UI state
  bool m_imguiInitialized = false;
  bool m_showDebugWindow = true;
  JAGEngine::ScreenState m_currentState = JAGEngine::ScreenState::NONE;
  bool m_isExiting = false;
  bool m_wasMouseDown = false;
  bool m_showNoStartLineMessage = false;
  float m_messageTimer = 0.0f;

  // Editor state backup
  bool m_savedObjectPlacementMode = false;
  int m_savedTemplateIndex = -1;
  bool m_showTrackingPoints = false;
  std::vector<std::pair<size_t, glm::vec2>> m_savedObjectStates;

  // Helper functions - grouped by functionality
  // UI functions
  void drawImGui();
  void checkImGuiState();
  void drawDebugWindow();
  void exitGame();
  glm::vec2 screenToWorld(const glm::vec2& screenCoords);

  // Track modification
  void updateRoadMesh();
  void initDefaultTrack();
  void validatePlacedObjects();
  glm::vec2 findClosestSplinePoint(const glm::vec2& mousePos);
  void addNodeAtPosition(const glm::vec2& position);
  void deleteSelectedNode();

  // Input handling
  void handleInput();
  void handleObjectPlacement(const glm::vec2& worldPos);

  // Test mode
  void initTestMode();
  void updateTestMode();
  void cleanupTestMode();
  void drawTestMode();
  void drawTestModeUI();
  void updateCarTrackingInfo();
  void updateTestCamera();
  void drawCarTrackingDebug();
  CarTrackingInfo calculateCarTrackingInfo(const Car* car) const;
  glm::vec2 calculateLookAheadPoint(const CarTrackingInfo& carInfo) const;

  using IGameScreen::m_game;
};
