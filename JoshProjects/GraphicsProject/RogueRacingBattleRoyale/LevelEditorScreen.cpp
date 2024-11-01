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
  //cleanupImGui();
}

void LevelEditorScreen::onEntry() {
  std::cout << "LevelEditorScreen::onEntry() start\n";

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

  std::cout << "LevelEditorScreen::onEntry() complete\n";
}

void LevelEditorScreen::onExit() {
  std::cout << "LevelEditorScreen::onExit() start\n";

  // Clear any selected nodes
  m_selectedNode = nullptr;
  m_isDragging = false;

  std::cout << "LevelEditorScreen::onExit() complete\n";
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
  ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);

  ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoCollapse);

  if (ImGui::Button("Back to Game", ImVec2(180, 40))) {
    m_currentState = JAGEngine::ScreenState::CHANGE_PREVIOUS;
  }

  if (ImGui::Button("Options", ImVec2(180, 40))) {
    std::cout << "Options clicked\n";
  }

  if (ImGui::Button("Exit Game", ImVec2(180, 40))) {
    exitGame();
  }

  ImGui::End();
}

void LevelEditorScreen::draw() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw game world
  m_program.use();

  GLint pUniform = m_program.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &m_projectionMatrix[0][0]);

  m_spriteBatch.begin();

  // Draw track spline points
  auto splinePoints = m_track->getSplinePoints(200);
  for (size_t i = 0; i < splinePoints.size() - 1; i += 2) {
    const auto& pointInfo = splinePoints[i];
    glm::vec4 destRect(
      pointInfo.position.x - 1.0f,
      pointInfo.position.y - 1.0f,
      2.0f,
      2.0f
    );
    JAGEngine::ColorRGBA8 white(255, 255, 255, 255);
    m_spriteBatch.draw(destRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, white);
  }

  // Draw road edges
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
      nodeColor = JAGEngine::ColorRGBA8(255, 0, 0, 255);
    }
    else if (node.isHovered()) {
      nodeColor = JAGEngine::ColorRGBA8(0, 150, 255, 255);
    }
    else {
      nodeColor = JAGEngine::ColorRGBA8(0, 255, 0, 255);
    }

    m_spriteBatch.draw(nodeRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, nodeColor);
  }

  // Draw preview node if in add mode
  if (m_addNodeMode && m_showPreviewNode) {
    glm::vec4 previewRect(
      m_previewNodePosition.x - 10.0f,
      m_previewNodePosition.y - 10.0f,
      20.0f,
      20.0f
    );
    JAGEngine::ColorRGBA8 previewColor(255, 255, 0, 200); // Yellow, semi-transparent
    m_spriteBatch.draw(previewRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, previewColor);
  }

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();

  m_program.unuse();

  // Draw ImGui windows
  drawImGui();
  drawDebugWindow();
}

// Update drawRoadEdges() to use SplinePointInfo correctly
void LevelEditorScreen::drawRoadEdges() {
  const auto& nodes = m_track->getNodes();
  if (nodes.empty()) return;

  // Get spline points with interpolated widths
  auto splinePoints = m_track->getSplinePoints(200);
  std::vector<glm::vec2> leftEdgePoints;
  std::vector<glm::vec2> rightEdgePoints;

  // Calculate perpendicular points for each spline point
  for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
    const auto& current = splinePoints[i];
    const auto& next = splinePoints[i + 1];

    // Calculate direction and perpendicular
    glm::vec2 direction = next.position - current.position;
    glm::vec2 perpendicular(-direction.y, direction.x);
    perpendicular = glm::normalize(perpendicular);

    // Use interpolated road width
    float roadWidth = current.roadWidth;

    // Calculate edge points using interpolated width
    leftEdgePoints.push_back(current.position + perpendicular * roadWidth);
    rightEdgePoints.push_back(current.position - perpendicular * roadWidth);
  }

  // Draw edges
  for (size_t i = 0; i < leftEdgePoints.size(); i += 2) {
    // Draw left edge
    glm::vec4 leftRect(
      leftEdgePoints[i].x - 1.0f,
      leftEdgePoints[i].y - 1.0f,
      2.0f,
      2.0f
    );
    JAGEngine::ColorRGBA8 edgeColor(255, 255, 255, 255);
    m_spriteBatch.draw(leftRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, edgeColor);

    // Draw right edge
    glm::vec4 rightRect(
      rightEdgePoints[i].x - 1.0f,
      rightEdgePoints[i].y - 1.0f,
      2.0f,
      2.0f
    );
    m_spriteBatch.draw(rightRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, edgeColor);
  }
}

void LevelEditorScreen::checkImGuiState() {
  if (!m_imguiInitialized) {
    std::cout << "ImGui not initialized!\n";
    return;
  }

  ImGuiIO& io = ImGui::GetIO();
  std::cout << "ImGui State:\n"
    << "  Initialized: " << m_imguiInitialized << "\n"
    << "  WantCaptureMouse: " << io.WantCaptureMouse << "\n"
    << "  WantCaptureKeyboard: " << io.WantCaptureKeyboard << "\n"
    << "  MousePos: " << io.MousePos.x << ", " << io.MousePos.y << "\n"
    << "  MouseDown[0]: " << io.MouseDown[0] << "\n"
    << "  MouseDown[1]: " << io.MouseDown[1] << "\n"
    << "  MouseWheel: " << io.MouseWheel << "\n";
}

void LevelEditorScreen::drawDebugWindow() {
  ImGui::SetNextWindowPos(ImVec2(10, 220), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);

  ImGui::Begin("Track Editor", nullptr, ImGuiWindowFlags_NoCollapse);

  if (ImGui::Button("Reset Track", ImVec2(180, 30))) {
    m_selectedNode = nullptr;
    m_isDragging = false;
    initDefaultTrack();
  }

  ImGui::Text("Nodes: %zu", m_track->getNodes().size());

  // Add node mode toggle
  ImGui::Checkbox("Add Node Mode", &m_addNodeMode);
  if (m_addNodeMode) {
    ImGui::TextWrapped("Click on the track to add a new node");
  }

  if (ImGui::CollapsingHeader("Node Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_selectedNode) {
      glm::vec2 pos = m_selectedNode->getPosition();
      ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);

      float width = m_selectedNode->getRoadWidth();
      if (ImGui::SliderFloat("Road Width##NodeWidth", &width, 10.0f, 100.0f)) {
        m_selectedNode->setRoadWidth(width);
      }

      // Add delete node button
      if (ImGui::Button("Delete Node", ImVec2(180, 30))) {
        deleteSelectedNode();
      }
    }
    else {
      ImGui::Text("No node selected");
    }
  }

  ImGui::End();
}


// Update handleInput() in LevelEditorScreen.cpp
void LevelEditorScreen::handleInput() {
  if (m_isExiting) return;

  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  JAGEngine::InputManager& inputManager = game->getInputManager();

  bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;
  if (imguiWantsMouse) return;

  // Convert screen coordinates to world coordinates
  float screenWidth = static_cast<float>(game->getWindow().getScreenWidth());
  float screenHeight = static_cast<float>(game->getWindow().getScreenHeight());

  glm::vec2 mousePos(inputManager.getMouseCoords());
  mousePos.x = ((mousePos.x / screenWidth) * 2.0f - 1.0f) * (screenWidth * 0.2f);
  mousePos.y = (-(mousePos.y / screenHeight) * 2.0f + 1.0f) * (screenHeight * 0.2f);

  // Get current mouse state
  bool isMouseDown = inputManager.isKeyDown(SDL_BUTTON_LEFT);

  if (m_addNodeMode) {
    // Update preview node position
    m_previewNodePosition = findClosestSplinePoint(mousePos);
    m_showPreviewNode = true;

    // Detect mouse button press (transition from up to down)
    if (isMouseDown && !m_wasMouseDown) {
      std::cout << "Mouse clicked! Adding node at position: "
        << m_previewNodePosition.x << ", "
        << m_previewNodePosition.y << std::endl;
      addNodeAtPosition(m_previewNodePosition);
    }
  }
  else {
    m_showPreviewNode = false;

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
    if (isMouseDown) {
      if (!m_isDragging) {
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
      m_isDragging = false;
    }
  }

  // Update previous mouse state
  m_wasMouseDown = isMouseDown;
}

void LevelEditorScreen::exitGame() {
  if (!m_isExiting) {
    m_isExiting = true;
    if (m_game) {
      m_game->exitGame();
    }
  }
}

void LevelEditorScreen::initDefaultTrack() {
  std::cout << "Creating new track...\n";
  if (m_track) {
    m_track = std::make_unique<SplineTrack>();
    m_track->createDefaultTrack();
    std::cout << "Track created with " << m_track->getNodes().size() << " nodes\n";
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

glm::vec2 LevelEditorScreen::findClosestSplinePoint(const glm::vec2& mousePos) {
  auto splinePoints = m_track->getSplinePoints(200);
  float minDist = std::numeric_limits<float>::max();
  glm::vec2 closestPoint;

  for (const auto& pointInfo : splinePoints) {
    float dist = glm::distance(mousePos, pointInfo.position);
    if (dist < minDist) {
      minDist = dist;
      closestPoint = pointInfo.position;
    }
  }

  return closestPoint;
}

void LevelEditorScreen::addNodeAtPosition(const glm::vec2& position) {
  std::cout << "Adding node at position: " << position.x << ", " << position.y << std::endl;

  auto& nodes = m_track->getNodes();
  if (nodes.empty()) {
    nodes.push_back(TrackNode(position));
    return;
  }

  // Find the nearest segment
  float minDist = std::numeric_limits<float>::max();
  size_t insertIndex = 0;

  for (size_t i = 0; i < nodes.size(); i++) {
    const auto& node1 = nodes[i];
    const auto& node2 = nodes[(i + 1) % nodes.size()];

    // Calculate point-to-line distance
    glm::vec2 segStart = node1.getPosition();
    glm::vec2 segEnd = node2.getPosition();
    glm::vec2 segDir = segEnd - segStart;
    float segLen = glm::length(segDir);

    if (segLen > 0) {
      segDir /= segLen;
      glm::vec2 toPoint = position - segStart;
      float proj = glm::dot(toPoint, segDir);
      proj = glm::clamp(proj, 0.0f, segLen);
      glm::vec2 closestPoint = segStart + segDir * proj;
      float dist = glm::distance(position, closestPoint);

      if (dist < minDist) {
        minDist = dist;
        insertIndex = (i + 1) % nodes.size();
      }
    }
  }

  // Create new node
  TrackNode newNode(position);

  // Interpolate road width from neighboring nodes
  float prevWidth = nodes[insertIndex > 0 ? insertIndex - 1 : nodes.size() - 1].getRoadWidth();
  float nextWidth = nodes[insertIndex].getRoadWidth();
  newNode.setRoadWidth((prevWidth + nextWidth) * 0.5f);

  // Insert the new node
  nodes.insert(nodes.begin() + insertIndex, newNode);
  std::cout << "Added node. New node count: " << nodes.size() << std::endl;
}

// Update deleteSelectedNode() in LevelEditorScreen.cpp
void LevelEditorScreen::deleteSelectedNode() {
  if (!m_selectedNode) {
    std::cout << "No node selected for deletion" << std::endl;
    return;
  }

  auto& nodes = m_track->getNodes();
  if (nodes.size() <= 4) {
    std::cout << "Cannot delete node: minimum node count reached" << std::endl;
    return;
  }

  // Find the selected node in the vector
  auto it = std::find_if(nodes.begin(), nodes.end(),
    [this](const TrackNode& node) {
      return &node == m_selectedNode;
    });

  if (it != nodes.end()) {
    std::cout << "Deleting node at index " << std::distance(nodes.begin(), it) << std::endl;
    nodes.erase(it);
    m_selectedNode = nullptr;
    m_isDragging = false;
    std::cout << "Node deleted. New node count: " << nodes.size() << std::endl;
  }
  else {
    std::cout << "Selected node not found in track nodes" << std::endl;
  }
}
