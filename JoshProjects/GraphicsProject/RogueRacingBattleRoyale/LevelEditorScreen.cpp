// LevelEditorScreen.cpp

#include "LevelEditorScreen.h"
#include "SplineTrack.h"
#include <JAGEngine/IMainGame.h>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL/SDL.h>
#include "imgui_impls.h"

LevelEditorScreen::LevelEditorScreen() {
}

LevelEditorScreen::~LevelEditorScreen() {
  destroy();
}

void LevelEditorScreen::build() {
}

void LevelEditorScreen::destroy() {
  cleanupImGui();
}

void LevelEditorScreen::onEntry() {
  SDL_Window* window = m_game->getWindow().getSDLWindow();
  SDL_GLContext gl_context = m_game->getWindow().getGLContext();
  m_imguiInitialized = InitImGui(window, gl_context);

  // Initialize rendering
  initShaders();
  m_spriteBatch.init();

  // Get window dimensions
  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  int screenWidth = game->getWindow().getScreenWidth();
  int screenHeight = game->getWindow().getScreenHeight();

  // Setup projection
  float zoom = 0.2f;
  m_projectionMatrix = glm::ortho(
    -screenWidth * zoom,
    screenWidth * zoom,
    -screenHeight * zoom,
    screenHeight * zoom,
    -1.0f, 1.0f
  );

  // Initialize track
  m_track = std::make_unique<SplineTrack>();
  initDefaultTrack();
}

void LevelEditorScreen::onExit() {
  cleanupImGui();
}

void LevelEditorScreen::update() {
  handleInput();
}

void LevelEditorScreen::initShaders() {
  // Compile shaders
  m_program.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
  m_program.addAttribute("vertexPosition");
  m_program.addAttribute("vertexColor");
  m_program.addAttribute("vertexUV");
  m_program.linkShaders();
}

void LevelEditorScreen::drawImGui() {
  // Main menu window
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);

  ImGui::Begin("Editor Menu", nullptr, ImGuiWindowFlags_NoCollapse);

  if (ImGui::Button("Main Menu", ImVec2(180, 40))) {
    m_currentState = JAGEngine::ScreenState::CHANGE_PREVIOUS;
  }

  if (ImGui::Button("Options", ImVec2(180, 40))) {
    // TODO: Implement options
    std::cout << "Options clicked\n";
  }

  if (ImGui::Button("Exit Game", ImVec2(180, 40))) {
    exitGame();
  }

  ImGui::Separator();
  ImGui::TextWrapped("Click and drag nodes to modify track shape");
  ImGui::Text("Press ESC to return to main menu");

  ImGui::End();
}

void LevelEditorScreen::draw() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Start ImGui frame
  NewImGuiFrame();

  // Draw track
  m_program.use();
  GLint pUniform = m_program.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &m_projectionMatrix[0][0]);

  m_spriteBatch.begin();

  // Draw spline points as small white dots - fixed color
  auto splinePoints = m_track->getSplinePoints(200);
  for (size_t i = 0; i < splinePoints.size() - 1; i += 2) { // Modified step for clearer dots
    const glm::vec2& point = splinePoints[i];
    // Draw each dot slightly larger
    glm::vec4 destRect(
      point.x - 1.0f,  // Slightly larger dot
      point.y - 1.0f,
      2.0f,
      2.0f
    );

    // Explicitly white color
    JAGEngine::ColorRGBA8 white;
    white.r = 255;
    white.g = 255;
    white.b = 255;
    white.a = 255;

    // Use texture coordinates that match the white part of your texture
    glm::vec4 uvRect(0.5f, 0.5f, 1.0f, 1.0f);  // Adjust these based on your texture

    m_spriteBatch.draw(destRect, uvRect, 0, 0.0f, white);
  }

  // Draw road edges here - we'll implement this next
  drawRoadEdges();

  // Draw nodes
  for (const auto& node : m_track->getNodes()) {
    glm::vec4 nodeRect(
      node.getPosition().x - 10.0f,
      node.getPosition().y - 10.0f,
      20.0f,
      20.0f
    );

    JAGEngine::ColorRGBA8 nodeColor;
    if (node.isSelected()) {
      // Red for selected
      nodeColor.r = 255;
      nodeColor.g = 0;
      nodeColor.b = 0;
      nodeColor.a = 255;
    }
    else if (node.isHovered()) {
      // Blue for hovered
      nodeColor.r = 0;
      nodeColor.g = 150;
      nodeColor.b = 255;
      nodeColor.a = 255;
    }
    else {
      // Green for normal
      nodeColor.r = 0;
      nodeColor.g = 255;
      nodeColor.b = 0;
      nodeColor.a = 255;
    }

    // Draw a slightly larger black outline first
    JAGEngine::ColorRGBA8 outlineColor(0, 0, 0, 255);
    glm::vec4 outlineRect(
      node.getPosition().x - 11.0f,
      node.getPosition().y - 11.0f,
      22.0f,
      22.0f
    );
    m_spriteBatch.draw(outlineRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, outlineColor);

    // Then draw the node
    m_spriteBatch.draw(nodeRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, nodeColor);
  }

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();

  m_program.unuse();

  // Draw ImGui windows
  drawMainMenu();
  drawDebugWindow();

  RenderImGui();
}

void LevelEditorScreen::drawDebugWindow() {
  if (!m_showDebugWindow) return;

  ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Track Editor", &m_showDebugWindow)) {
    if (ImGui::Button("Reset Track", ImVec2(180, 30))) {
      initDefaultTrack();
    }

    ImGui::Text("Nodes: %zu", m_track->getNodes().size());

    if (m_selectedNode) {
      ImGui::Separator();
      ImGui::Text("Selected Node Properties:");
      glm::vec2 pos = m_selectedNode->getPosition();
      ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);

      // Road width control with wider range and live feedback
      float roadWidth = m_selectedNode->getRoadWidth();
      if (ImGui::SliderFloat("Road Width", &roadWidth, 10.0f, 100.0f, "%.1f")) {
        m_selectedNode->setRoadWidth(roadWidth);
      }

      // Add a visual indicator of the current width
      ImGui::Text("Current Width: %.1f", m_selectedNode->getRoadWidth());
    }
  }
  ImGui::End();
}

void LevelEditorScreen::handleInput() {
  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  JAGEngine::InputManager& inputManager = game->getInputManager();

  // If ImGui is handling input, don't process game input
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  // Convert screen coordinates to world coordinates
  float screenWidth = static_cast<float>(game->getWindow().getScreenWidth());
  float screenHeight = static_cast<float>(game->getWindow().getScreenHeight());

  glm::vec2 mousePos(inputManager.getMouseCoords());
  mousePos.x = ((mousePos.x / screenWidth) * 2.0f - 1.0f) * (screenWidth * 0.2f);
  mousePos.y = (-(mousePos.y / screenHeight) * 2.0f + 1.0f) * (screenHeight * 0.2f);

  // Reset hover states
  for (auto& node : m_track->getNodes()) {
    node.setHovered(false);
  }

  // Check for node under cursor
  TrackNode* hoveredNode = m_track->getNodeAtPosition(mousePos, 20.0f);
  if (hoveredNode) {
    hoveredNode->setHovered(true);
  }

  // Handle node selection and dragging
  if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
    if (!m_isDragging) {
      // Only change selection if we click on a node
      if (hoveredNode) {
        // Deselect previous node if it's different
        if (m_selectedNode && m_selectedNode != hoveredNode) {
          m_selectedNode->setSelected(false);
        }
        m_selectedNode = hoveredNode;
        m_selectedNode->setSelected(true);
        m_isDragging = true;
      }
    }

    // Move the node if we're dragging
    if (m_isDragging && m_selectedNode) {
      m_selectedNode->setPosition(mousePos);
    }
  }
  else {
    // Just stop dragging when mouse is released, maintain selection
    m_isDragging = false;
  }

  m_lastMousePos = mousePos;
}

void LevelEditorScreen::exitGame() {
  if (m_game) {
    m_game->exitGame();
  }
}

void LevelEditorScreen::drawMainMenu() {
  // Editor menu window - fixed position in top-left
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);

  ImGui::Begin("Editor Menu", nullptr, ImGuiWindowFlags_NoCollapse);

  // Use available width instead of GetWindowContentRegionWidth
  float windowWidth = ImGui::GetContentRegionAvail().x;

  if (ImGui::Button("Main Menu", ImVec2(windowWidth, 40))) {
    m_currentState = JAGEngine::ScreenState::CHANGE_PREVIOUS;
  }

  ImGui::Spacing();
  if (ImGui::Button("Options", ImVec2(windowWidth, 40))) {
    std::cout << "Options clicked\n";
  }

  ImGui::Spacing();
  if (ImGui::Button("Exit Game", ImVec2(windowWidth, 40))) {
    exitGame();
  }

  ImGui::End();
}

void LevelEditorScreen::drawRoadEdges() {
  const auto& nodes = m_track->getNodes();
  if (nodes.empty()) return;

  // Get spline points for both edges
  auto splinePoints = m_track->getSplinePoints(200);
  std::vector<glm::vec2> leftEdgePoints;
  std::vector<glm::vec2> rightEdgePoints;

  // Calculate perpendicular points for each spline point
  for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
    glm::vec2 current = splinePoints[i];
    glm::vec2 next = splinePoints[i + 1];

    // Calculate direction and perpendicular
    glm::vec2 direction = next - current;
    glm::vec2 perpendicular(-direction.y, direction.x);
    perpendicular = glm::normalize(perpendicular);

    // Interpolate road width between nodes
    float t = static_cast<float>(i) / splinePoints.size();
    size_t nodeIndex = static_cast<size_t>(t * (nodes.size() - 1));
    float roadWidth = nodes[nodeIndex].getRoadWidth();

    // Calculate edge points
    leftEdgePoints.push_back(current + perpendicular * roadWidth);
    rightEdgePoints.push_back(current - perpendicular * roadWidth);
  }

  // Draw left edge
  for (size_t i = 0; i < leftEdgePoints.size(); i += 2) {
    glm::vec4 destRect(
      leftEdgePoints[i].x - 1.0f,
      leftEdgePoints[i].y - 1.0f,
      2.0f,
      2.0f
    );
    JAGEngine::ColorRGBA8 edgeColor(255, 255, 255, 255);
    m_spriteBatch.draw(destRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, edgeColor);
  }

  // Draw right edge
  for (size_t i = 0; i < rightEdgePoints.size(); i += 2) {
    glm::vec4 destRect(
      rightEdgePoints[i].x - 1.0f,
      rightEdgePoints[i].y - 1.0f,
      2.0f,
      2.0f
    );
    JAGEngine::ColorRGBA8 edgeColor(255, 255, 255, 255);
    m_spriteBatch.draw(destRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, edgeColor);
  }
}

void LevelEditorScreen::initDefaultTrack() {
  m_track->createDefaultTrack();
}

void LevelEditorScreen::cleanupImGui() {
  if (m_imguiInitialized) {
    ShutdownImGui();
    m_imguiInitialized = false;
  }
}

// Screen transition methods
int LevelEditorScreen::getNextScreenIndex() const {
  return -1;  // No next screen
}

// And LevelEditorScreen's getPreviousScreenIndex:
int LevelEditorScreen::getPreviousScreenIndex() const {
  return 0;  // Index of GameplayScreen
}
