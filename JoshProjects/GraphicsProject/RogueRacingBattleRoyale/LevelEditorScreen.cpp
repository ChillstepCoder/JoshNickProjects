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

  initShaders();
  m_spriteBatch.init();

  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  int screenWidth = game->getWindow().getScreenWidth();
  int screenHeight = game->getWindow().getScreenHeight();

  m_camera.init(screenWidth, screenHeight);
  m_camera.setScale(1.0f);
  m_camera.setPosition(glm::vec2(0.0f));

  // Initialize track
  m_track = std::make_unique<SplineTrack>();
  initDefaultTrack();

  // Initialize shaders
  m_roadShader.compileShaders("Shaders/road.vert", "Shaders/road.frag");
  m_roadShader.addAttribute("vertexPosition");
  m_roadShader.addAttribute("vertexUV");
  m_roadShader.addAttribute("distanceAlong");
  m_roadShader.linkShaders();

  m_offroadShader.compileShaders("Shaders/offroad.vert", "Shaders/offroad.frag");
  m_offroadShader.addAttribute("vertexPosition");
  m_offroadShader.addAttribute("vertexUV");
  m_offroadShader.addAttribute("distanceAlong");
  m_offroadShader.linkShaders();

// Initialize grass shader
  m_grassShader.compileShaders("Shaders/grass.vert", "Shaders/grass.frag");
  m_grassShader.addAttribute("vertexPosition");
  m_grassShader.addAttribute("vertexUV");
  m_grassShader.linkShaders();

  // Initialize background quad
  initBackgroundQuad();

  // Initialize meshes
  updateRoadMesh();

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

void LevelEditorScreen::initBackgroundQuad() {
  RoadVertex v1, v2, v3, v4;
  // Use even larger size to ensure coverage
  float size = 20000.0f;  // Increased size
  v1.position = glm::vec2(-size, -size);
  v2.position = glm::vec2(size, -size);
  v3.position = glm::vec2(size, size);
  v4.position = glm::vec2(-size, size);

  // UV coordinates for tiling
  v1.uv = glm::vec2(0.0f, 0.0f);
  v2.uv = glm::vec2(1.0f, 0.0f);
  v3.uv = glm::vec2(1.0f, 1.0f);
  v4.uv = glm::vec2(0.0f, 1.0f);

  m_backgroundQuad.vertices = { v1, v2, v3, v4 };
  m_backgroundQuad.indices = { 0, 1, 2, 2, 3, 0 };

  RoadMeshGenerator::createBuffers(m_backgroundQuad);
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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Update camera
  m_camera.update();
  glm::mat4 cameraMatrix = m_camera.getCameraMatrix();

  // Draw grass background
  m_grassShader.use();

  glUniformMatrix4fv(m_grassShader.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  glUniform3fv(m_grassShader.getUniformLocation("grassColor"), 1, &m_grassColor[0]);
  glUniform1f(m_grassShader.getUniformLocation("noiseScale"), m_grassNoiseScale);
  glUniform1f(m_grassShader.getUniformLocation("noiseIntensity"), m_grassNoiseIntensity);

  glBindVertexArray(m_backgroundQuad.vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);

  m_grassShader.unuse();

  if (m_roadViewMode == RoadViewMode::Shaded) {
    // Draw offroad first (it goes under the road)
    m_offroadShader.use();
    GLint offroadColorUniform = m_offroadShader.getUniformLocation("offroadColor");
    if (offroadColorUniform != -1) {
      glUniform3fv(offroadColorUniform, 1, &m_offroadColor[0]);
    }

    GLint offroadPUniform = m_offroadShader.getUniformLocation("P");
    glUniformMatrix4fv(offroadPUniform, 1, GL_FALSE, &(cameraMatrix[0][0]));

    // Draw left offroad
    if (m_offroadMesh.leftSide.vao != 0) {
      glBindVertexArray(m_offroadMesh.leftSide.vao);
      glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(m_offroadMesh.leftSide.indices.size()),
        GL_UNSIGNED_INT, nullptr);
    }

    // Draw right offroad
    if (m_offroadMesh.rightSide.vao != 0) {
      glBindVertexArray(m_offroadMesh.rightSide.vao);
      glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(m_offroadMesh.rightSide.indices.size()),
        GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);
    m_offroadShader.unuse();

    // Draw the shaded road
    m_roadShader.use();
    GLint roadPUniform = m_roadShader.getUniformLocation("P");
    glUniformMatrix4fv(roadPUniform, 1, GL_FALSE, &(cameraMatrix[0][0]));

    if (m_roadMesh.vao != 0 && !m_roadMesh.indices.empty()) {
      glBindVertexArray(m_roadMesh.vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_roadMesh.ibo);
      glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(m_roadMesh.indices.size()),
        GL_UNSIGNED_INT,
        nullptr);
      glBindVertexArray(0);
    }
    m_roadShader.unuse();
  }

  // Draw spline points and control nodes if enabled
  if (m_showSplinePoints || m_roadViewMode == RoadViewMode::Spline) {
    m_program.use();
    GLint spritePUniform = m_program.getUniformLocation("P");
    glUniformMatrix4fv(spritePUniform, 1, GL_FALSE, &(cameraMatrix[0][0]));

    m_spriteBatch.begin();

    // Draw spline points if in spline mode
    if (m_roadViewMode == RoadViewMode::Spline) {
      drawRoadEdges();
    }

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
      JAGEngine::ColorRGBA8 previewColor(255, 255, 0, 200);
      m_spriteBatch.draw(previewRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, previewColor);
    }

    m_spriteBatch.end();
    m_spriteBatch.renderBatch();

    m_program.unuse();
  }

  // Draw wireframe mode
  if (m_roadViewMode == RoadViewMode::Wireframe) {
    m_roadShader.use();
    GLint roadPUniform = m_roadShader.getUniformLocation("P");
    glUniformMatrix4fv(roadPUniform, 1, GL_FALSE, &(cameraMatrix[0][0]));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Draw road mesh
    if (m_roadMesh.vao != 0 && !m_roadMesh.indices.empty()) {
      glBindVertexArray(m_roadMesh.vao);
      glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(m_roadMesh.indices.size()),
        GL_UNSIGNED_INT,
        nullptr);
    }

    // Draw offroad meshes in a different color
    m_offroadShader.use();
    GLint offroadPUniform = m_offroadShader.getUniformLocation("P");
    glUniformMatrix4fv(offroadPUniform, 1, GL_FALSE, &(cameraMatrix[0][0]));

    // Left offroad
    if (m_offroadMesh.leftSide.vao != 0) {
      glBindVertexArray(m_offroadMesh.leftSide.vao);
      glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(m_offroadMesh.leftSide.indices.size()),
        GL_UNSIGNED_INT,
        nullptr);
    }

    // Right offroad
    if (m_offroadMesh.rightSide.vao != 0) {
      glBindVertexArray(m_offroadMesh.rightSide.vao);
      glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(m_offroadMesh.rightSide.indices.size()),
        GL_UNSIGNED_INT,
        nullptr);
    }

    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    m_offroadShader.unuse();
  }

  // Draw ImGui windows last
  drawImGui();
  drawDebugWindow();
}

// Update drawRoadEdges() to use SplinePointInfo correctly
void LevelEditorScreen::drawRoadEdges() {
  const auto& nodes = m_track->getNodes();
  if (nodes.empty()) return;

  auto splinePoints = m_track->getSplinePoints(200);
  std::vector<glm::vec2> leftEdgePoints, rightEdgePoints;
  std::vector<glm::vec2> leftOffroadPoints, rightOffroadPoints;

  // Calculate points for each spline point
  for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
    const auto& current = splinePoints[i];
    const auto& next = splinePoints[i + 1];

    // Calculate direction and perpendicular
    glm::vec2 direction = next.position - current.position;
    glm::vec2 perpendicular = glm::normalize(glm::vec2(-direction.y, direction.x));

    // Road edges
    leftEdgePoints.push_back(current.position + perpendicular * current.roadWidth);
    rightEdgePoints.push_back(current.position - perpendicular * current.roadWidth);

    // Offroad edges
    leftOffroadPoints.push_back(current.position + perpendicular * (current.roadWidth + current.offroadWidth.x));
    rightOffroadPoints.push_back(current.position - perpendicular * (current.roadWidth + current.offroadWidth.y));
  }

  // Draw all edges
  JAGEngine::ColorRGBA8 roadEdgeColor(255, 255, 255, 255);
  JAGEngine::ColorRGBA8 offroadEdgeColor(200, 150, 50, 255);

  // Draw road edges
  for (size_t i = 0; i < leftEdgePoints.size(); i += 2) {
    // Left edge
    glm::vec4 leftRect(leftEdgePoints[i].x - 1.0f, leftEdgePoints[i].y - 1.0f, 2.0f, 2.0f);
    m_spriteBatch.draw(leftRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, roadEdgeColor);

    // Right edge
    glm::vec4 rightRect(rightEdgePoints[i].x - 1.0f, rightEdgePoints[i].y - 1.0f, 2.0f, 2.0f);
    m_spriteBatch.draw(rightRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, roadEdgeColor);

    // Left offroad edge
    glm::vec4 leftOffroadRect(leftOffroadPoints[i].x - 1.0f, leftOffroadPoints[i].y - 1.0f, 2.0f, 2.0f);
    m_spriteBatch.draw(leftOffroadRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, offroadEdgeColor);

    // Right offroad edge
    glm::vec4 rightOffroadRect(rightOffroadPoints[i].x - 1.0f, rightOffroadPoints[i].y - 1.0f, 2.0f, 2.0f);
    m_spriteBatch.draw(rightOffroadRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, offroadEdgeColor);
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

  // View Options
  if (ImGui::CollapsingHeader("View Options", ImGuiTreeNodeFlags_DefaultOpen)) {
    const char* viewModes[] = { "Spline View", "Wireframe", "Shaded Road" };
    int currentMode = static_cast<int>(m_roadViewMode);
    if (ImGui::Combo("View Mode", &currentMode, viewModes, IM_ARRAYSIZE(viewModes))) {
      m_roadViewMode = static_cast<RoadViewMode>(currentMode);
    }

    ImGui::Checkbox("Show Control Points", &m_showSplinePoints);

    // LOD Controls
    ImGui::Separator();
    ImGui::Text("Road Detail Level");

    // LOD slider with real-time preview
    if (ImGui::SliderInt("Subdivisions##LOD", &m_roadLOD, MIN_LOD, MAX_LOD)) {
      if (m_autoUpdateMesh) {
        updateRoadMesh();
      }
    }

    ImGui::Checkbox("Auto Update Mesh", &m_autoUpdateMesh);
    if (!m_autoUpdateMesh && ImGui::Button("Update Mesh", ImVec2(180, 25))) {
      updateRoadMesh();
    }

    // Show vertex/triangle count
    if (m_roadMesh.vertices.size() > 0) {
      ImGui::Text("Vertices: %zu", m_roadMesh.vertices.size());
      ImGui::Text("Triangles: %zu", m_roadMesh.indices.size() / 3);
    }
  }

  // Add node mode toggle
  ImGui::Checkbox("Add Node Mode", &m_addNodeMode);
  if (m_addNodeMode) {
    ImGui::TextWrapped("Click on the track to add a new node");
  }

  // Node Properties
  if (ImGui::CollapsingHeader("Node Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_selectedNode) {
      glm::vec2 pos = m_selectedNode->getPosition();
      ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);

      float width = m_selectedNode->getRoadWidth();
      if (ImGui::SliderFloat("Road Width##NodeWidth", &width, 10.0f, 100.0f)) {
        m_selectedNode->setRoadWidth(width);
        updateRoadMesh();
      }

      glm::vec2 offroadWidth = m_selectedNode->getOffroadWidth();
      if (ImGui::SliderFloat2("Offroad Width##NodeOffroad", &offroadWidth.x, 0.0f, 100.0f)) {
        m_selectedNode->setOffroadWidth(offroadWidth);
        updateRoadMesh();
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

  // In LevelEditorScreen::drawDebugWindow(), find the Terrain Colors section
  if (ImGui::CollapsingHeader("Terrain Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool colorChanged = false;

    // Grass color control
    float grassColor[3] = { m_grassColor.r, m_grassColor.g, m_grassColor.b };
    if (ImGui::ColorEdit3("Grass Color", grassColor)) {
      m_grassColor = glm::vec3(grassColor[0], grassColor[1], grassColor[2]);
    }

    // Grass noise controls with wider ranges
    ImGui::SliderFloat("Grass Pattern Scale", &m_grassNoiseScale, 0.1f, 500.0f, "%.1f");
    ImGui::SliderFloat("Grass Pattern Intensity", &m_grassNoiseIntensity, 0.0f, 1.0f, "%.3f");

    if (ImGui::Button("Reset Grass Pattern", ImVec2(180, 25))) {
      m_grassNoiseScale = 50.0f;    // Default scale
      m_grassNoiseIntensity = 0.3f; // Default intensity
    }

    // Offroad color control
    float offroadColor[3] = { m_offroadColor.r, m_offroadColor.g, m_offroadColor.b };
    if (ImGui::ColorEdit3("Offroad Color", offroadColor)) {
      m_offroadColor = glm::vec3(offroadColor[0], offroadColor[1], offroadColor[2]);
    }
  }

  ImGui::End();
}

glm::vec2 LevelEditorScreen::screenToWorld(const glm::vec2& screenCoords) {
  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  float screenWidth = static_cast<float>(game->getWindow().getScreenWidth());
  float screenHeight = static_cast<float>(game->getWindow().getScreenHeight());

  // Get current camera scale and position
  float scale = m_camera.getScale();
  glm::vec2 cameraPos = m_camera.getPosition();

  // Convert screen coordinates to world coordinates
  glm::vec2 worldCoords;
  worldCoords.x = ((screenCoords.x / screenWidth) * 2.0f - 1.0f) * (screenWidth * 0.2f) / scale;
  worldCoords.y = (-(screenCoords.y / screenHeight) * 2.0f + 1.0f) * (screenHeight * 0.2f) / scale;

  // Apply camera position
  worldCoords += cameraPos;

  return worldCoords;
}

void LevelEditorScreen::handleInput() {
  if (m_isExiting) return;

  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  JAGEngine::InputManager& inputManager = game->getInputManager();

  bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;
  bool imguiWantsKeyboard = ImGui::GetIO().WantCaptureKeyboard;

  // Handle camera controls if ImGui isn't capturing keyboard
  if (!imguiWantsKeyboard) {
    // Handle zoom
    float currentScale = m_camera.getScale();

    if (inputManager.isKeyDown(SDLK_q) || inputManager.isKeyDown(SDLK_MINUS) ||
      inputManager.isKeyDown(SDLK_UNDERSCORE)) {
      currentScale /= m_zoomFactor;
    }
    if (inputManager.isKeyDown(SDLK_e) || inputManager.isKeyDown(SDLK_EQUALS) ||
      inputManager.isKeyDown(SDLK_PLUS)) {
      currentScale *= m_zoomFactor;
    }

    // Clamp zoom
    currentScale = glm::clamp(currentScale, m_minZoom, m_maxZoom);
    m_camera.setScale(currentScale);

    // Handle camera movement
    glm::vec2 cameraPosition = m_camera.getPosition();
    float adjustedSpeed = m_cameraSpeed;

    if (inputManager.isKeyDown(SDLK_w) || inputManager.isKeyDown(SDLK_UP)) {
      cameraPosition.y += adjustedSpeed;
    }
    if (inputManager.isKeyDown(SDLK_s) || inputManager.isKeyDown(SDLK_DOWN)) {
      cameraPosition.y -= adjustedSpeed;
    }
    if (inputManager.isKeyDown(SDLK_a) || inputManager.isKeyDown(SDLK_LEFT)) {
      cameraPosition.x -= adjustedSpeed;
    }
    if (inputManager.isKeyDown(SDLK_d) || inputManager.isKeyDown(SDLK_RIGHT)) {
      cameraPosition.x += adjustedSpeed;
    }

    m_camera.setPosition(cameraPosition);
  }

  // Handle mouse input
  if (!imguiWantsMouse) {
    // Convert screen coordinates to world coordinates
    glm::vec2 screenCoords = inputManager.getMouseCoords();
    glm::vec2 worldPos = m_camera.convertScreenToWorld(screenCoords);

    // Debug output
    std::cout << "Screen coords: (" << screenCoords.x << ", " << screenCoords.y << ")\n";
    std::cout << "World coords: (" << worldPos.x << ", " << worldPos.y << ")\n";
    std::cout << "Camera pos: (" << m_camera.getPosition().x << ", " << m_camera.getPosition().y
      << ") Scale: " << m_camera.getScale() << "\n";

    if (m_addNodeMode) {
      m_previewNodePosition = findClosestSplinePoint(worldPos);
      m_showPreviewNode = true;

      // Debug preview position
      std::cout << "Preview pos: (" << m_previewNodePosition.x << ", "
        << m_previewNodePosition.y << ")\n";

      if (inputManager.isKeyDown(SDL_BUTTON_LEFT) && !m_wasMouseDown) {
        addNodeAtPosition(m_previewNodePosition);
      }
    }
    else {
      m_showPreviewNode = false;

      // Reset hover states
      for (auto& node : m_track->getNodes()) {
        node.setHovered(false);
      }

      // Scale selection radius with camera zoom
      float baseRadius = 20.0f;
      float scaledRadius = baseRadius / m_camera.getScale();

      TrackNode* hoveredNode = m_track->getNodeAtPosition(worldPos, scaledRadius);
      if (hoveredNode) {
        hoveredNode->setHovered(true);
        glm::vec2 nodePos = hoveredNode->getPosition();
        std::cout << "Hovering node at: (" << nodePos.x << ", " << nodePos.y << ")\n";
      }

      if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
        if (!m_isDragging && hoveredNode) {
          if (m_selectedNode && m_selectedNode != hoveredNode) {
            m_selectedNode->setSelected(false);
          }
          m_selectedNode = hoveredNode;
          m_selectedNode->setSelected(true);
          m_isDragging = true;
        }

        if (m_isDragging && m_selectedNode) {
          m_selectedNode->setPosition(worldPos);
          updateRoadMesh();
        }
      }
      else {
        m_isDragging = false;
      }
    }

    m_wasMouseDown = inputManager.isKeyDown(SDL_BUTTON_LEFT);
  }
}


void LevelEditorScreen::exitGame() {
  if (!m_isExiting) {
    m_isExiting = true;
    if (m_game) {
      m_game->exitGame();
    }
  }
}

void LevelEditorScreen::updateRoadMesh() {
  std::cout << "Updating road meshes...\n";

  // Clean up old meshes
  m_roadMesh.cleanup();
  m_offroadMesh.leftSide.cleanup();
  m_offroadMesh.rightSide.cleanup();

  // Generate new meshes
  m_roadMesh = RoadMeshGenerator::generateRoadMesh(*m_track, m_roadLOD);
  m_offroadMesh = RoadMeshGenerator::generateOffroadMesh(*m_track, m_roadLOD);
}

void LevelEditorScreen::initDefaultTrack() {
  std::cout << "Creating new track...\n";
  if (m_track) {
    m_track = std::make_unique<SplineTrack>();
    m_track->createDefaultTrack();
    std::cout << "Track created with " << m_track->getNodes().size() << " nodes\n";
  }
}

void LevelEditorScreen::drawMeshDebug() {
  if (m_roadViewMode == RoadViewMode::Wireframe) {
    // Draw vertices as small points
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (const auto& vertex : m_roadMesh.vertices) {
      if (vertex.uv.x == 0.5f) {
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow for center points
      }
      else {
        glColor3f(1.0f, 0.0f, 0.0f); // Red for edge points
      }
      glVertex2f(vertex.position.x, vertex.position.y);
    }
    glEnd();
    glPointSize(1.0f);
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

  updateRoadMesh();

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
    updateRoadMesh();
    std::cout << "Node deleted. New node count: " << nodes.size() << std::endl;
  }
  else {
    std::cout << "Selected node not found in track nodes" << std::endl;
  }


}
