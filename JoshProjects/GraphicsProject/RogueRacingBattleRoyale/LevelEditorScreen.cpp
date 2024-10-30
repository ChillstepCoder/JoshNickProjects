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
  std::cout << "LevelEditorScreen::onEntry() start\n";  // Debug output

  // Initialize ImGui from scratch
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  SDL_Window* window = m_game->getWindow().getSDLWindow();
  SDL_GLContext gl_context = m_game->getWindow().getGLContext();

  bool imguiInitSuccess = ImGui_ImplSDL2_InitForOpenGL(window, gl_context) &&
    ImGui_ImplOpenGL3_Init("#version 130");

  std::cout << "ImGui initialization " << (imguiInitSuccess ? "succeeded" : "failed") << "\n";
  m_imguiInitialized = imguiInitSuccess;

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
  std::cout << "LevelEditorScreen::onExit() start\n";  // Debug output

  // Clean up ImGui
  if (m_imguiInitialized) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    m_imguiInitialized = false;
  }

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
  if (m_isExiting || !m_imguiInitialized) {
    std::cout << "Skipping draw: " << (m_isExiting ? "exiting" : "imgui not initialized") << "\n";
    return;
  }

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Start ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  // Debug output before drawing windows
  checkImGuiState();

  // Draw main menu first to ensure it's on top
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);

  ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

  // Fixed color pushing with correct ImGui enum
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));

  if (ImGui::Button("Back to Game", ImVec2(180, 40))) {
    std::cout << "Back to Game clicked\n";
    m_currentState = JAGEngine::ScreenState::CHANGE_PREVIOUS;
  }

  if (ImGui::Button("Options", ImVec2(180, 40))) {
    std::cout << "Options clicked\n";
  }

  if (ImGui::Button("Exit Game", ImVec2(180, 40))) {
    std::cout << "Exit clicked\n";
    exitGame();
  }

  ImGui::PopStyleColor();
  ImGui::End();

  // Draw debug window
  drawDebugWindow();

  // Draw game world after ImGui windows
  m_program.use();
  GLint pUniform = m_program.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &m_projectionMatrix[0][0]);

  m_spriteBatch.begin();

  // Draw track spline points
  auto splinePoints = m_track->getSplinePoints(200);
  for (size_t i = 0; i < splinePoints.size() - 1; i += 2) {
    const glm::vec2& point = splinePoints[i];
    glm::vec4 destRect(point.x - 1.0f, point.y - 1.0f, 2.0f, 2.0f);
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

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();
  m_program.unuse();

  // End ImGui frame
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Debug output after frame
  checkImGuiState();
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

  if (ImGui::CollapsingHeader("Node Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_selectedNode) {
      glm::vec2 pos = m_selectedNode->getPosition();
      ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);

      float width = m_selectedNode->getRoadWidth();
      if (ImGui::SliderFloat("Road Width##NodeWidth", &width, 10.0f, 100.0f)) {
        m_selectedNode->setRoadWidth(width);
        std::cout << "Width changed to: " << width << std::endl;
      }
    }
    else {
      ImGui::Text("No node selected");
    }
  }

  ImGui::End();
}


void LevelEditorScreen::handleInput() {
  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  JAGEngine::InputManager& inputManager = game->getInputManager();

  // Process SDL events first
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    std::cout << "SDL Event Type: " << event.type << std::endl;

    if (m_imguiInitialized) {
      bool handled = ImGui_ImplSDL2_ProcessEvent(&event);
      std::cout << "ImGui " << (handled ? "handled" : "ignored") << " event\n";
    }

    if (event.type == SDL_QUIT) {
      exitGame();
      return;
    }
  }

  // Check ImGui state
  bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;
  bool imguiWantsKeyboard = ImGui::GetIO().WantCaptureKeyboard;

  std::cout << "ImGui wants: Mouse=" << imguiWantsMouse
    << " Keyboard=" << imguiWantsKeyboard << "\n";

  // Only process game input if ImGui isn't capturing it
  if (imguiWantsMouse) {
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
    std::cout << "Node hovered at: " << hoveredNode->getPosition().x << ", "
      << hoveredNode->getPosition().y << std::endl;
  }

  // Handle node selection and dragging
  if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
    if (!m_isDragging) {
      if (hoveredNode) {
        // Deselect previous node if it's different
        if (m_selectedNode && m_selectedNode != hoveredNode) {
          m_selectedNode->setSelected(false);
        }
        m_selectedNode = hoveredNode;
        m_selectedNode->setSelected(true);
        m_isDragging = true;
        std::cout << "Node selected with road width: " << m_selectedNode->getRoadWidth() << std::endl;
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

void LevelEditorScreen::exitGame() {
  if (!m_isExiting) {
    m_isExiting = true;
    cleanupImGui();
    if (m_game) {
      m_game->exitGame();
    }
  }
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
  std::cout << "Creating new track...\n";
  if (m_track) {
    m_track = std::make_unique<SplineTrack>();
    m_track->createDefaultTrack();
    std::cout << "Track created with " << m_track->getNodes().size() << " nodes\n";
  }
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
