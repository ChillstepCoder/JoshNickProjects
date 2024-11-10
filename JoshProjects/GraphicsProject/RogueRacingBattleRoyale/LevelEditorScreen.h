// LevelEditorScreen.h

#pragma once
#include <JAGEngine/IGameScreen.h>
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/Camera2D.h>
#include "ObjectManager.h"
#include "RoadMeshGenerator.h"
#include <glm/glm.hpp>
#include <vector>
#include <ImGui/imgui.h>
#include "LevelRenderer.h"


// Forward declarations
class SplineTrack;
class TrackNode;


class LevelEditorScreen : public JAGEngine::IGameScreen {
public:
  enum class RoadViewMode {
    Spline,      // Shows spline points and control nodes
    Wireframe,   // Shows triangle mesh wireframe
    Shaded       // Shows fully shaded road
  };

  LevelEditorScreen();
  ~LevelEditorScreen();

  virtual void build() override;
  virtual void destroy() override;
  virtual void onEntry() override;
  virtual void onExit() override;
  virtual void update() override;
  virtual void draw() override;
  virtual int getNextScreenIndex() const override;
  virtual int getPreviousScreenIndex() const override;

private:

  glm::vec2 screenToWorld(const glm::vec2& screenCoords);
  void drawImGui();
  void checkImGuiState();
  void drawDebugWindow();
  void exitGame();
  void handleInput();
  void handleObjectPlacement(const glm::vec2& worldPos);
  void updateRoadMesh();
  void initDefaultTrack();
  void validatePlacedObjects();

  glm::vec2 findClosestSplinePoint(const glm::vec2& mousePos);
  void addNodeAtPosition(const glm::vec2& position);
  void deleteSelectedNode();

  // Track editing
  std::unique_ptr<SplineTrack> m_track;
  TrackNode* m_selectedNode = nullptr;
  TrackNode* m_draggedNode = nullptr;
  bool m_isDragging = false;
  glm::vec2 m_lastMousePos;

  bool m_addNodeMode = false;
  bool m_deleteObjectMode = false;
  glm::vec2 m_previewNodePosition = glm::vec2(0.0f);
  bool m_showPreviewNode = false;

  // Rendering
  JAGEngine::SpriteBatch m_spriteBatch;
  glm::mat4 m_projectionMatrix;
  std::unique_ptr<LevelRenderer> m_levelRenderer;

  // ImGui state
  bool m_imguiInitialized = false;
  bool m_showDebugWindow = true;

  JAGEngine::ScreenState m_currentState = JAGEngine::ScreenState::NONE;
  bool m_isExiting = false;

  bool m_wasMouseDown = false;

  JAGEngine::Camera2D m_camera;
  float m_cameraSpeed = 8.0f;
  float m_zoomFactor = 1.02f;
  float m_minZoom = 0.1f;
  float m_maxZoom = 5.0f;

  using IGameScreen::m_game;

  // Shading

  glm::vec3 m_grassColor = glm::vec3(0.2f, 0.5f, 0.1f);
  glm::vec3 m_offroadColor = glm::vec3(0.45f, 0.32f, 0.15f);
  float m_grassNoiseScale = 100.0f;
  float m_grassNoiseIntensity = 1.0f;

  glm::vec3 m_barrierPrimaryColor = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 m_barrierSecondaryColor = glm::vec3(0.0f, 0.0f, 0.0f);
  float m_barrierPatternScale = 5.0f;

  RoadViewMode m_roadViewMode = RoadViewMode::Shaded;
  bool m_showSplinePoints = true;

  int m_roadLOD = 10;            // Default subdivisions
  const int MIN_LOD = 4;         // Minimum subdivisions
  const int MAX_LOD = 50;        // Maximum subdivisions
  bool m_autoUpdateMesh = true;  // Auto update when LOD changes

  std::unique_ptr<ObjectManager> m_objectManager;
  int m_selectedTemplateIndex = -1;
  bool m_objectPlacementMode = false;
  PlaceableObject* m_selectedObject = nullptr;
  bool m_isDraggingObject = false;
  glm::vec2 m_lastDragPos;

};
