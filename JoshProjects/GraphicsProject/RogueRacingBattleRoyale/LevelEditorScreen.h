// LevelEditorScreen.h

#pragma once
#include <JAGEngine/IGameScreen.h>
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
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
  void initShaders();
  void cleanupImGui();
  void drawImGui();
  void drawMainMenu();
  void drawDebugWindow();
  void exitGame();
  void handleInput();
  void drawRoadEdges();
  void initDefaultTrack();

  // Track editing
  std::unique_ptr<SplineTrack> m_track;
  TrackNode* m_selectedNode = nullptr;
  TrackNode* m_draggedNode = nullptr;
  bool m_isDragging = false;
  glm::vec2 m_lastMousePos;

  // Rendering
  JAGEngine::SpriteBatch m_spriteBatch;
  JAGEngine::GLSLProgram m_program;
  glm::mat4 m_projectionMatrix;

  // ImGui state
  bool m_imguiInitialized = false;
  bool m_showDebugWindow = true;
};
