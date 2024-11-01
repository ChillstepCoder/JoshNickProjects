// LevelEditorScreen.h

#pragma once
#include <JAGEngine/IGameScreen.h>
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/Camera2D.h>
#include <glm/glm.hpp>
#include <vector>
#include <ImGui/imgui.h>

// Forward declarations
class SplineTrack;
class TrackNode;

class LevelEditorScreen : public JAGEngine::IGameScreen {
public:
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
  void initShaders();
  void drawImGui();
  void checkImGuiState();
  void drawDebugWindow();
  void exitGame();
  void handleInput();
  void drawRoadEdges();
  void initDefaultTrack();

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
  glm::vec2 m_previewNodePosition = glm::vec2(0.0f);
  bool m_showPreviewNode = false;

  // Rendering
  JAGEngine::SpriteBatch m_spriteBatch;
  JAGEngine::GLSLProgram m_program;
  glm::mat4 m_projectionMatrix;

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

  using IGameScreen::m_game;  // Make m_game accessible
};
