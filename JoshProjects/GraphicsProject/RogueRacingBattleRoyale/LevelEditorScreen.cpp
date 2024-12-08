// LevelEditorScreen.cpp

// GLM configuration
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_PURE
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_CXX17

// Standard library includes
#include <limits>
#include <algorithm>
#include <iostream>
#include <regex>
#include <cstring>
#include <numeric>
#include <cfloat>

// GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

// SDL includes
#include <SDL/SDL.h>

// ImGui includes
#include "imgui_impls.h"

// Engine includes
#include <JAGEngine/IMainGame.h>

// Local includes
#include "LevelEditorScreen.h"
#include "SplineTrack.h"

template<typename T>
T getMin(T a, T b) {
  return (a < b) ? a : b;
}

template<typename T>
T getMax(T a, T b) {
  return (a > b) ? a : b;
}

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
  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  int screenWidth = game->getWindow().getScreenWidth();
  int screenHeight = game->getWindow().getScreenHeight();

  // Initialize Shaders
  m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
  m_textureProgram.addAttribute("vertexPosition");
  m_textureProgram.addAttribute("vertexColor");
  m_textureProgram.addAttribute("vertexUV");
  m_textureProgram.linkShaders();

  // Initialize cameras
  m_camera.init(screenWidth, screenHeight);
  m_camera.setScale(1.0f);
  m_camera.setPosition(glm::vec2(0.0f));

  // HUD
  m_hudCamera.init(screenWidth, screenHeight);
  m_hudCamera.setPosition(glm::vec2(screenWidth / 2.0f, screenHeight / 2.0f));
  m_hudCamera.setScale(1.0f);

  // Initialize sprite batches
  m_spriteBatch.init();
  m_hudSpriteBatch.init();

  // Initialize track and renderer components
  m_track = std::make_unique<SplineTrack>();
  m_track->createDefaultTrack();

  m_levelRenderer = std::make_unique<LevelRenderer>();
  m_levelRenderer->init();
  m_levelRenderer->setShowStartPositions(m_showStartPositions);

  m_objectManager = std::make_unique<ObjectManager>(m_track.get(), nullptr);
  m_objectManager->createDefaultTemplates();

  m_levelRenderer->updateRoadMesh(m_track.get());

  // Initialize text rendering
  try {
    // Initialize shaders first
    m_textRenderingProgram.compileShaders("Shaders/textRendering.vert", "Shaders/textRendering.frag");
    m_textRenderingProgram.addAttribute("vertexPosition");
    m_textRenderingProgram.addAttribute("vertexColor");
    m_textRenderingProgram.addAttribute("vertexUV");
    m_textRenderingProgram.linkShaders();

    // Check uniforms
    GLint pUniform = m_textRenderingProgram.getUniformLocation("P");
    GLint samplerUniform = m_textRenderingProgram.getUniformLocation("mySampler");
    if (pUniform == -1 || samplerUniform == -1) {
      throw std::runtime_error("Failed to get shader uniforms");
    }

    // Initialize font after shader validation
    m_countdownFont = std::make_unique<JAGEngine::SpriteFont>();
    m_countdownFont->init("Fonts/titilium_bold.ttf", 72);

    // Initialize countdown with same font
    m_raceCountdown = std::make_unique<RaceCountdown>();
    m_raceCountdown->setFont(m_countdownFont.get());

    m_raceCountdown->setOnCountdownStart([this]() {
      m_enableAI = false;
      m_inputEnabled = false;
      });

    m_raceCountdown->setOnCountdownComplete([this]() {
      m_enableAI = true;
      m_inputEnabled = true;
      });

    std::cout << "Text Rendering Setup Complete:\n"
      << "- Shader Program ID: " << m_textRenderingProgram.getProgramID() << "\n"
      << "- Countdown Font Texture ID: " << m_countdownFont->getTextureID() << "\n"
      << "- P uniform location: " << pUniform << "\n"
      << "- Sampler uniform location: " << samplerUniform << "\n";
  }
  catch (const std::exception& e) {
    std::cerr << "Text rendering initialization error: " << e.what() << std::endl;
  }

  std::cout << "LevelEditorScreen::onEntry() complete\n";
}

void LevelEditorScreen::onExit() {
  std::cout << "LevelEditorScreen::onExit() start\n";

  // Clear any selected nodes
  m_selectedNode = nullptr;
  m_isDragging = false;

  // Cleanup level renderer
  if (m_levelRenderer) {
    m_levelRenderer->destroy();
  }

  // Clean up test mode resources
  cleanupTestMode();

  // Clean up text rendering resources
  m_countdownFont.reset();
  m_raceCountdown.reset();

  std::cout << "LevelEditorScreen::onExit() complete\n";
}

void LevelEditorScreen::update() {

  if (m_showNoStartLineMessage) {
    m_messageTimer -= 1.0f / 60.0f; // Assuming 60 FPS
    if (m_messageTimer <= 0) {
      m_showNoStartLineMessage = false;
    }
  }

  if (m_testMode) {
    updateTestMode();
  }
  else {
    handleInput();
  }
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

  // Separate debug window
  ImGui::SetNextWindowPos(ImVec2(220, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);
  ImGui::Begin("OpenGL Debug", nullptr, ImGuiWindowFlags_NoCollapse);

  ImGui::Text("OpenGL Errors: %d", JAGEngine::OpenGLDebug::GetErrorCount());

  if (ImGui::Button("Test Invalid Texture")) {
    glBindTexture(GL_TEXTURE_2D, 99999);
  }

  if (ImGui::Button("Test Invalid Enum")) {
    glEnable(GL_INVALID_ENUM);
}

  if (ImGui::Button("Test Invalid Draw")) {
    glDrawArrays(GL_TRIANGLES, 0, -1);
  }

  if (ImGui::Button("Reset Error Count")) {
    JAGEngine::OpenGLDebug::ResetErrorCount();
  }

  ImGui::End();
}

void LevelEditorScreen::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Update camera
  m_camera.update();

  // Check if level renderer exists before using it
  if (m_levelRenderer) {
    // Update level renderer settings from GUI state
    try {
      m_levelRenderer->setGrassColor(m_grassColor);
      m_levelRenderer->setOffroadColor(m_offroadColor);
      m_levelRenderer->setGrassNoiseParams(m_grassNoiseScale, m_grassNoiseIntensity);
      m_levelRenderer->setBarrierColors(m_barrierPrimaryColor, m_barrierSecondaryColor);
      m_levelRenderer->setBarrierPatternScale(m_barrierPatternScale);
      m_levelRenderer->setRoadLOD(m_roadLOD);

      // Automatically hide start positions in test mode
      if (m_testMode) {
        m_levelRenderer->setShowStartPositions(false);
      }

      // Add back this critical preview setup code
      // Set preview node if in add node mode
      if (m_addNodeMode) {
        m_levelRenderer->setPreviewNode(m_previewNodePosition, m_showPreviewNode);
      }
      else {
        m_levelRenderer->setPreviewNode(glm::vec2(0.0f), false);
      }

      // Object placement preview
      m_levelRenderer->setObjectPlacementMode(m_objectPlacementMode,
        m_selectedTemplateIndex,
        m_testMode);  // Pass test mode state
      if (m_objectPlacementMode && !m_testMode) {  // Only show preview if not in test mode
        glm::vec2 mousePos = m_camera.convertScreenToWorld(
          m_game->getInputManager().getMouseCoords());
        m_levelRenderer->setPreviewPosition(mousePos);
      }

      bool showSplinePoints = m_showSplinePoints;
      if (m_testMode) showSplinePoints = false;

      // Main render call
      if (m_track) {
        m_levelRenderer->render(m_camera,
          m_track.get(),
          m_objectManager.get(),
          showSplinePoints,
          static_cast<LevelRenderer::RoadViewMode>(m_roadViewMode));

        // Draw start positions after main render, but only if not in test mode
        if (m_levelRenderer->getShowStartPositions() && !m_testMode) {
          m_levelRenderer->renderStartPositions(m_camera.getCameraMatrix(), m_track.get());
        }
      }
    }
    catch (const std::exception& e) {
      std::cerr << "Error in renderer: " << e.what() << std::endl;
    }
  }

  // Draw test mode cars if in test mode
  if (m_testMode) {
    drawTestMode();
    drawTestModeUI();
  }

  // Draw ImGui windows last, but only if not in test mode
  if (!m_testMode) {
    drawImGui();
    drawDebugWindow();
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
  if (m_testMode) return;

  ImGui::SetNextWindowPos(ImVec2(10, 220), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);

  ImGui::Begin("Track Editor", nullptr, ImGuiWindowFlags_NoCollapse);

  // Add difficulty selector near the top
  ImGui::Text("Level Difficulty");
  ImGui::SliderInt("##Difficulty", &m_levelDifficulty, 1, 10);

  // Add save/load buttons
  if (ImGui::Button("Save Level", ImVec2(180, 30))) {
    m_showSavePrompt = true;
    memset(m_levelNameBuffer, 0, sizeof(m_levelNameBuffer));
  }

  if (ImGui::Button("Load Level", ImVec2(180, 30))) {
    m_showLoadPrompt = true;
    m_availableLevels = LevelSaveLoad::getLevelList();
    m_selectedLevelIndex = -1;
  }

  if (ImGui::CollapsingHeader("Test Mode", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Test Level", ImVec2(180, 30))) {
      initTestMode();
    }
  }

  if (ImGui::Button("Reset Track", ImVec2(180, 30))) {
    m_selectedNode = nullptr;
    m_isDragging = false;
    initDefaultTrack();
  }

  // Add null check before accessing track
  if (m_track) {
    ImGui::Text("Nodes: %zu", m_track->getNodes().size());
  }
  else {
    ImGui::Text("Track not initialized");
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
  }

  // Add node mode toggle
  if (ImGui::Checkbox("Add Node Mode", &m_addNodeMode)) {
    if (m_addNodeMode) {
      m_objectPlacementMode = false;
      m_deleteObjectMode = false;
    }
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

      // Starting line
      bool isStartLine = m_selectedNode->isStartLine();
      if (ImGui::Checkbox("Start Line##NodeStart", &isStartLine)) {
        if (isStartLine) {
          m_track->setStartLine(m_selectedNode);
        }
        else {
          m_track->setStartLine(nullptr);
        }
        updateRoadMesh();
      }

      if (m_selectedNode->isStartLine()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
          "This node is the start/finish line");
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

  // Barrier control
  if (ImGui::CollapsingHeader("Barrier Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Barrier colors
    float primaryColor[3] = { m_barrierPrimaryColor.r, m_barrierPrimaryColor.g, m_barrierPrimaryColor.b };
    if (ImGui::ColorEdit3("Primary Color", primaryColor)) {
      m_barrierPrimaryColor = glm::vec3(primaryColor[0], primaryColor[1], primaryColor[2]);
    }

    float secondaryColor[3] = { m_barrierSecondaryColor.r, m_barrierSecondaryColor.g, m_barrierSecondaryColor.b };
    if (ImGui::ColorEdit3("Secondary Color", secondaryColor)) {
      m_barrierSecondaryColor = glm::vec3(secondaryColor[0], secondaryColor[1], secondaryColor[2]);
    }

    // Pattern scale
    ImGui::SliderFloat("Pattern Scale", &m_barrierPatternScale, 1.0f, 10.0f);

    // If a node is selected, show barrier distance controls
    if (m_selectedNode) {
      glm::vec2 barrierDist = m_selectedNode->getBarrierDistance();
      if (ImGui::SliderFloat2("Barrier Distance##NodeBarrier", &barrierDist.x, 0.0f, 100.0f)) {
        m_selectedNode->setBarrierDistance(barrierDist);
        updateRoadMesh();
      }
    }
  }

  // Object Placement
  if (ImGui::CollapsingHeader("Object Placement", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Checkbox("Place Objects", &m_objectPlacementMode)) {
      if (m_objectPlacementMode) {
        m_addNodeMode = false;
        m_deleteObjectMode = false;
      }
    }
    // Delete Objects Mode
    if (ImGui::Checkbox("Delete Objects", &m_deleteObjectMode)) {
      if (m_deleteObjectMode) {
        m_addNodeMode = false;
        m_objectPlacementMode = false;
      }
    }
    const auto& templates = m_objectManager->getObjectTemplates();

    // Create preview label
    std::string previewValue = (m_selectedTemplateIndex >= 0) ?
      templates[m_selectedTemplateIndex]->getDisplayName() :
      "Select Object";

    // Object type combo
    if (ImGui::BeginCombo("Object Type", previewValue.c_str())) {
      for (size_t i = 0; i < templates.size(); i++) {
        bool isSelected = (m_selectedTemplateIndex == i);
        if (ImGui::Selectable(templates[i]->getDisplayName().c_str(), isSelected)) {
          m_selectedTemplateIndex = i;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    // Show placement zone info
    if (m_selectedTemplateIndex >= 0) {
      const auto& selectedTemplate = templates[m_selectedTemplateIndex];
      const char* zoneText = "";
      switch (selectedTemplate->getZone()) {
      case PlacementZone::Road:
        zoneText = "Can only be placed on road";
        break;
      case PlacementZone::Offroad:
        zoneText = "Can only be placed on offroad areas";
        break;
      case PlacementZone::Grass:
        zoneText = "Can only be placed on grass";
        break;
      case PlacementZone::Anywhere:
        zoneText = "Can be placed anywhere";
        break;
      }
      ImGui::TextWrapped("%s", zoneText);
    }
  }

  // Terrain Colors section
  if (ImGui::CollapsingHeader("Terrain Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Grass color control
    float grassColor[3] = { m_grassColor.r, m_grassColor.g, m_grassColor.b };
    if (ImGui::ColorEdit3("Grass Color", grassColor)) {
      m_grassColor = glm::vec3(grassColor[0], grassColor[1], grassColor[2]);
    }

    // Grass noise controls
    ImGui::SliderFloat("Grass Pattern Scale", &m_grassNoiseScale, 0.1f, 500.0f, "%.1f");
    ImGui::SliderFloat("Grass Pattern Intensity", &m_grassNoiseIntensity, 0.0f, 1.0f, "%.3f");

    if (ImGui::Button("Reset Grass Pattern", ImVec2(180, 25))) {
      m_grassNoiseScale = 50.0f;
      m_grassNoiseIntensity = 0.3f;
    }

    // Offroad color control
    float offroadColor[3] = { m_offroadColor.r, m_offroadColor.g, m_offroadColor.b };
    if (ImGui::ColorEdit3("Offroad Color", offroadColor)) {
      m_offroadColor = glm::vec3(offroadColor[0], offroadColor[1], offroadColor[2]);
    }
  }

  // Start Line Configuration

  if (ImGui::CollapsingHeader("Start Line Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
    SplineTrack::StartPositionConfig& config = m_track->getStartPositionConfig();

    // Update checkbox label to be more clear
    if (ImGui::Checkbox("Clockwise Direction (Default is Counter-clockwise)", &config.isClockwise)) {
      updateRoadMesh();
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Counter-clockwise is the default direction.\nToggle to change racing direction.");
    }

    if (ImGui::SliderInt("Number of Cars", &config.numPositions, 2, 20)) {
      config.numPositions = glm::clamp(config.numPositions, 2, 20);
      updateRoadMesh();
    }

    if (ImGui::SliderInt("Number of Lanes", &config.numLanes, 1, 4)) {
      config.numLanes = glm::clamp(config.numLanes, 1, 4);
      updateRoadMesh();
    }

    if (ImGui::DragFloat("Car Spacing", &config.carSpacing, 0.5f, 20.0f, 150.0f)) {
      updateRoadMesh();
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Distance between rows of cars");
    }

    // Visualization toggle
    m_showStartPositions = m_levelRenderer->getShowStartPositions();
    if (ImGui::Checkbox("Show Start Positions", &m_showStartPositions)) {
      m_levelRenderer->setShowStartPositions(m_showStartPositions);
    }

    // Start line info
    TrackNode* startNode = m_track->getStartLineNode();
    if (startNode) {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
        "Start line is set at node position (%.1f, %.1f)",
        startNode->getPosition().x,
        startNode->getPosition().y);

      // Get the track direction at the start node
      glm::vec2 direction = m_track->getTrackDirectionAtNode(startNode);

      // Adjust direction for clockwise tracks if needed
      if (config.isClockwise) {
        direction = -direction;
      }

      ImGui::Text("Track Direction: (%.2f, %.2f)", direction.x, direction.y);
    }
    else {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
        "No start line set! Select a node and mark it as start line.");
    }

    // Show starting position information
    if (ImGui::TreeNode("Starting Position Info")) {
      auto positions = m_track->calculateStartPositions();
      for (size_t i = 0; i < positions.size(); i++) {
        ImGui::Text("Car %zu: Position (%.1f, %.1f) Angle: %.1f°",
          i + 1,
          positions[i].position.x,
          positions[i].position.y,
          glm::degrees(positions[i].angle));
      }
      ImGui::TreePop();
    }
  }

  if (m_showNoStartLineMessage) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 200.0f,
      ImGui::GetIO().DisplaySize.y * 0.5f - 50.0f));
    ImGui::SetNextWindowSize(ImVec2(400, 100));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Start Line Required", nullptr, flags)) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
      ImGui::TextWrapped("You must set a start line before testing the track!");
      ImGui::TextWrapped("Select a node and check 'Start Line' in the Node Properties panel.");
      ImGui::PopStyleColor();
    }
    ImGui::End();
  }

  ImGui::End();

  // Draw save prompt modal
  if (m_showSavePrompt) {
    drawSaveModal();
  }

  // Draw load prompt modal
  if (m_showLoadPrompt) {
    drawLoadModal();
  }
}

void LevelEditorScreen::drawSaveModal() {
  // Check if this is first time showing the modal
  static bool initNameBuffer = false;
  if (!initNameBuffer && m_showSavePrompt) {
    // Initialize buffer with current level name if loading from an existing file
    if (!m_loadedFilename.empty()) {
      // Extract name from filename (e.g. "1_mytrack.txt" -> "mytrack")
      std::regex levelPattern(R"(\d+_([^\.]+)\.txt)");
      std::smatch matches;
      if (std::regex_match(m_loadedFilename, matches, levelPattern)) {
        // Fixed the strncpy call to include size as third parameter
        strncpy_s(m_levelNameBuffer, matches[1].str().c_str(), sizeof(m_levelNameBuffer) - 1);
        m_levelNameBuffer[sizeof(m_levelNameBuffer) - 1] = '\0';
      }
    }
    initNameBuffer = true;
  }

  ImGui::OpenPopup("Save Level##modal");
  ImGui::SetNextWindowSize(ImVec2(300, 120));
  if (ImGui::BeginPopupModal("Save Level##modal", nullptr, ImGuiWindowFlags_NoResize)) {
    ImGui::Text("Enter level name:");
    ImGui::InputText("##levelname", m_levelNameBuffer, sizeof(m_levelNameBuffer));

    if (ImGui::Button("Save", ImVec2(120, 0))) {
      if (strlen(m_levelNameBuffer) > 0) {
        saveLevelToFile();
        m_loadedFilename = LevelSaveLoad::constructFilename(m_levelDifficulty, m_levelNameBuffer);
        m_showSavePrompt = false;
        initNameBuffer = false;
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      m_showSavePrompt = false;
      initNameBuffer = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void LevelEditorScreen::drawLoadModal() {
  ImGui::OpenPopup("Load Level##modal");
  ImGui::SetNextWindowSize(ImVec2(300, 400));
  if (ImGui::BeginPopupModal("Load Level##modal", nullptr, ImGuiWindowFlags_NoResize)) {
    ImGui::Text("Select a level to load:");
    ImGui::Separator();

    ImGui::BeginChild("LevelList", ImVec2(0, 300), true);
    for (size_t i = 0; i < m_availableLevels.size(); i++) {
      const auto& level = m_availableLevels[i];
      std::string label = "Difficulty " + std::to_string(level.difficulty) +
        ": " + level.levelName;

      if (ImGui::Selectable(label.c_str(), m_selectedLevelIndex == static_cast<int>(i))) {
        m_selectedLevelIndex = static_cast<int>(i);
      }
    }
    ImGui::EndChild();

    if (ImGui::Button("Load", ImVec2(120, 0))) {
      if (m_selectedLevelIndex >= 0 &&
        m_selectedLevelIndex < static_cast<int>(m_availableLevels.size())) {
        loadLevelFromFile(m_availableLevels[m_selectedLevelIndex].filename);
        m_showLoadPrompt = false;
        ImGui::CloseCurrentPopup();
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      m_showLoadPrompt = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void LevelEditorScreen::saveLevelToFile() {
  LevelSaveLoad::SavedLevel level;
  level.difficulty = m_levelDifficulty;
  level.name = m_levelNameBuffer;

  // Save track nodes
  level.nodes = m_track->getNodes();

  // Save placed objects
  const auto& placedObjects = m_objectManager->getPlacedObjects();
  const auto& templates = m_objectManager->getObjectTemplates();

  for (const auto& obj : placedObjects) {
    size_t templateIndex = 0;
    for (size_t i = 0; i < templates.size(); i++) {
      if (templates[i]->getDisplayName() == obj->getDisplayName()) {
        templateIndex = i;
        break;
      }
    }
    level.objects.push_back({ templateIndex, obj->getPosition() });
  }

  // Save track settings
  level.startConfig = m_track->getStartPositionConfig();
  level.grassColor = m_grassColor;
  level.offroadColor = m_offroadColor;
  level.grassNoiseScale = m_grassNoiseScale;
  level.grassNoiseIntensity = m_grassNoiseIntensity;
  level.barrierPrimaryColor = m_barrierPrimaryColor;
  level.barrierSecondaryColor = m_barrierSecondaryColor;
  level.barrierPatternScale = m_barrierPatternScale;

  if (LevelSaveLoad::saveLevel(level)) {
    m_loadedFilename = LevelSaveLoad::constructFilename(m_levelDifficulty, m_levelNameBuffer);
    std::cout << "Level saved successfully\n";
  }
  else {
    std::cout << "Failed to save level\n";
  }
}

void LevelEditorScreen::loadLevelFromFile(const std::string& filename) {
  LevelSaveLoad::SavedLevel loadedLevel;
  if (LevelSaveLoad::loadLevel(filename, loadedLevel)) {
    // Store the filename
    m_loadedFilename = filename;
    // Apply loaded level data
    m_track = std::make_unique<SplineTrack>();
    m_track->getNodes() = loadedLevel.nodes;

    // Clear existing objects
    m_objectManager = std::make_unique<ObjectManager>(m_track.get(), nullptr);
    m_objectManager->createDefaultTemplates();

    // Restore objects
    for (const auto& obj : loadedLevel.objects) {
      m_objectManager->addObject(obj.templateIndex, obj.position);
    }

    // Restore track settings
    m_track->setStartPositionConfig(loadedLevel.startConfig);
    m_grassColor = loadedLevel.grassColor;
    m_offroadColor = loadedLevel.offroadColor;
    m_grassNoiseScale = loadedLevel.grassNoiseScale;
    m_grassNoiseIntensity = loadedLevel.grassNoiseIntensity;
    m_barrierPrimaryColor = loadedLevel.barrierPrimaryColor;
    m_barrierSecondaryColor = loadedLevel.barrierSecondaryColor;
    m_barrierPatternScale = loadedLevel.barrierPatternScale;

    // Update difficulty
    m_levelDifficulty = loadedLevel.difficulty;

    // Update meshes
    updateRoadMesh();

    std::cout << "Level loaded successfully\n";
  }
  else {
    std::cout << "Failed to load level\n";
  }
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

  // Handle zoom for Editor Mode
  if (!m_testMode && !imguiWantsKeyboard) {
    // Editor mode zoom controls
    float currentScale = m_camera.getScale();
    if (inputManager.isKeyDown(SDLK_q)) {
      currentScale /= m_zoomFactor;
      m_camera.setScale(glm::clamp(currentScale, m_minZoom, m_maxZoom));
    }
    if (inputManager.isKeyDown(SDLK_e)) {
      currentScale *= m_zoomFactor;
      m_camera.setScale(glm::clamp(currentScale, m_minZoom, m_maxZoom));
    }
  }

  // Camera movement in editor mode
  if (!m_testMode && !imguiWantsKeyboard && m_canMoveCamera) {
    glm::vec2 cameraMove(0.0f);
    float deltaTime = 1.0f / 60.0f;

    // Scale camera speed based on zoom level
    float zoomScale = m_camera.getScale();
    float adjustedSpeed = m_cameraSpeed * (1.0f / zoomScale);

    if (inputManager.isKeyDown(SDLK_w)) cameraMove.y += adjustedSpeed * deltaTime;
    if (inputManager.isKeyDown(SDLK_s)) cameraMove.y -= adjustedSpeed * deltaTime;
    if (inputManager.isKeyDown(SDLK_a)) cameraMove.x -= adjustedSpeed * deltaTime;
    if (inputManager.isKeyDown(SDLK_d)) cameraMove.x += adjustedSpeed * deltaTime;

    if (glm::length(cameraMove) > 0) {
      m_camera.setPosition(m_camera.getPosition() + cameraMove);
    }
  }

  // ====================================
  // Handle mouse input
  // ====================================
  if (!imguiWantsMouse) {
    glm::vec2 screenCoords = inputManager.getMouseCoords();
    glm::vec2 worldPos = m_camera.convertScreenToWorld(screenCoords);

    // -----------------------------
    // Handle different modes
    // -----------------------------
    if (m_objectPlacementMode) {
      // Object placement mode - handle placing new objects
      handleObjectPlacement(worldPos);
    }
    else if (m_deleteObjectMode) {
      // Delete mode - remove objects on click
      if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
        PlaceableObject* clickedObject = m_objectManager->getObjectAtPosition(worldPos);
        if (clickedObject) {
          if (m_selectedObject) m_selectedObject->setSelected(false);
          m_selectedObject = clickedObject;
          m_selectedObject->setSelected(true);
          m_objectManager->removeSelectedObject();
          m_selectedObject = nullptr;
        }
      }
    }
    else if (m_addNodeMode) {
      // Add node mode - preview and add new track nodes
      m_previewNodePosition = findClosestSplinePoint(worldPos);
      m_showPreviewNode = true;

      if (inputManager.isKeyDown(SDL_BUTTON_LEFT) && !m_wasMouseDown) {
        addNodeAtPosition(m_previewNodePosition);
      }

      // Clear any selected objects in add node mode
      if (m_selectedObject) {
        m_selectedObject->setSelected(false);
        m_selectedObject = nullptr;
      }
      m_isDraggingObject = false;
    }
    else {
      // -----------------------------
      // Free manipulation mode
      // -----------------------------

      // Update node hover states if not dragging an object
      if (!m_isDraggingObject) {
        for (auto& node : m_track->getNodes()) {
          node.setHovered(false);
        }

        float baseRadius = 20.0f;
        float scaledRadius = baseRadius / m_camera.getScale();
        TrackNode* hoveredNode = m_track->getNodeAtPosition(worldPos, scaledRadius);
        if (hoveredNode) {
          hoveredNode->setHovered(true);
        }

        // Handle node selection and dragging
        if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
          if (!m_isDragging && hoveredNode && !m_isDraggingObject) {
            // Select new node
            if (m_selectedNode && m_selectedNode != hoveredNode) {
              m_selectedNode->setSelected(false);
            }
            m_selectedNode = hoveredNode;
            m_selectedNode->setSelected(true);
            m_isDragging = true;
          }

          if (m_isDragging && m_selectedNode) {
            // Move selected node
            m_selectedNode->setPosition(worldPos);
            updateRoadMesh();
          }
        }
        else {
          // Handle node drag release
          if (m_isDragging) {
            validatePlacedObjects();
          }
          m_isDragging = false;
        }
      }

      // Handle object manipulation
      if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
        if (!m_isDraggingObject && !m_isDragging) {
          // Try to select an object if not already dragging anything
          PlaceableObject* clickedObject = m_objectManager->getObjectAtPosition(worldPos);
          if (clickedObject) {
            // Clear any selected node when starting object drag
            if (m_selectedNode) {
              m_selectedNode->setSelected(false);
              m_selectedNode = nullptr;
            }

            // Select the object
            if (m_selectedObject) m_selectedObject->setSelected(false);
            m_selectedObject = clickedObject;
            m_selectedObject->setSelected(true);
            m_isDraggingObject = true;
            m_lastDragPos = worldPos;
          }
          else {
            if (m_selectedObject) {
              m_selectedObject->setSelected(false);
              m_selectedObject = nullptr;
            }
          }
        }
        else if (m_isDraggingObject && m_selectedObject) {
          // Continue dragging selected object
          glm::vec2 dragDelta = worldPos - m_lastDragPos;
          glm::vec2 newPos = m_selectedObject->getPosition() + dragDelta;

          // Update object position and show preview
          m_selectedObject->setPosition(newPos);
          m_lastDragPos = worldPos;

          bool validPlacement = m_objectManager->isValidPlacement(m_selectedObject, newPos);
          m_levelRenderer->setObjectPlacementPreview(true, m_selectedObject, newPos);
        }
      }
      else {
        // Handle object drag release
        if (m_isDraggingObject && m_selectedObject) {
          glm::vec2 finalPos = m_selectedObject->getPosition();
          if (!m_objectManager->isValidPlacement(m_selectedObject, finalPos)) {
            m_objectManager->removeSelectedObject();
            m_selectedObject = nullptr;
          }
          m_isDraggingObject = false;
          m_levelRenderer->setObjectPlacementPreview(false, nullptr, glm::vec2(0.0f));
        }
      }
    }

    m_wasMouseDown = inputManager.isKeyDown(SDL_BUTTON_LEFT);
  }
}

void LevelEditorScreen::handleObjectPlacement(const glm::vec2& worldPos) {
  if (!m_objectManager) return;

  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  JAGEngine::InputManager& inputManager = game->getInputManager();

  if (m_selectedTemplateIndex >= 0) {
    const auto& templates = m_objectManager->getObjectTemplates();
    if (m_selectedTemplateIndex < templates.size()) {
      const auto& template_obj = templates[m_selectedTemplateIndex];

      // Place object on mouse click
      if (inputManager.isKeyDown(SDL_BUTTON_LEFT) && !m_wasMouseDown) {
        if (m_objectManager->isValidPlacement(template_obj.get(), worldPos)) {
          m_objectManager->addObject(m_selectedTemplateIndex, worldPos);
        }
      }
    }
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
  if (m_levelRenderer) {
    m_levelRenderer->updateRoadMesh(m_track.get());
  }
}

void LevelEditorScreen::initDefaultTrack() {
  std::cout << "Creating new track...\n";
  if (m_track) {
    m_track = std::make_unique<SplineTrack>();
    m_track->createDefaultTrack();
    std::cout << "Track created with " << m_track->getNodes().size() << " nodes\n";

    updateRoadMesh();
  }
}

void LevelEditorScreen::validatePlacedObjects() {
  if (!m_objectManager) return;

  std::cout << "Validating placed objects after track modification...\n";

  std::vector<PlaceableObject*> objectsToRemove;
  const auto& placedObjects = m_objectManager->getPlacedObjects();

  // First pass: identify invalid objects
  for (const auto& obj : placedObjects) {
    if (!m_objectManager->isValidPlacement(obj.get(), obj->getPosition())) {
      objectsToRemove.push_back(obj.get());
      std::cout << "Object at position (" << obj->getPosition().x << ", "
        << obj->getPosition().y << ") is now invalid\n";
    }
  }

  // Second pass: remove invalid objects
  if (!objectsToRemove.empty()) {
    std::cout << "Removing " << objectsToRemove.size() << " invalid objects\n";
    m_objectManager->removeInvalidObjects(objectsToRemove);
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
  float minDist = FLT_MAX;
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
    TrackNode newNode(position);
    newNode.setBarrierDistance(glm::vec2(10.0f, 10.0f)); // Set default barrier distance
    nodes.push_back(newNode);
    return;
  }

  // Find the nearest segment
  float minDist = FLT_MAX;
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

  // Create new node with interpolated values
  TrackNode newNode(position);

  // Interpolate properties from neighboring nodes
  const TrackNode& prevNode = nodes[insertIndex > 0 ? insertIndex - 1 : nodes.size() - 1];
  const TrackNode& nextNode = nodes[insertIndex];

  // Interpolate road width
  float prevWidth = prevNode.getRoadWidth();
  float nextWidth = nextNode.getRoadWidth();
  newNode.setRoadWidth((prevWidth + nextWidth) * 0.5f);

  // Interpolate offroad width
  glm::vec2 prevOffroadWidth = prevNode.getOffroadWidth();
  glm::vec2 nextOffroadWidth = nextNode.getOffroadWidth();
  newNode.setOffroadWidth((prevOffroadWidth + nextOffroadWidth) * 0.5f);

  // Interpolate barrier distance with minimum value
  glm::vec2 prevBarrierDist = prevNode.getBarrierDistance();
  glm::vec2 nextBarrierDist = nextNode.getBarrierDistance();
  glm::vec2 interpolatedBarrierDist = (prevBarrierDist + nextBarrierDist) * 0.5f;

  // Ensure minimum barrier distance
  interpolatedBarrierDist.x = std::max<float>(interpolatedBarrierDist.x, 10.0f);
  interpolatedBarrierDist.y = std::max<float>(interpolatedBarrierDist.y, 10.0f);
  newNode.setBarrierDistance(interpolatedBarrierDist);

  // Insert the new node
  nodes.insert(nodes.begin() + insertIndex, newNode);
  updateRoadMesh();
  validatePlacedObjects();

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
    validatePlacedObjects();

    std::cout << "Node deleted. New node count: " << nodes.size() << std::endl;
  }
  else {
    std::cout << "Selected node not found in track nodes" << std::endl;
  }


}

void LevelEditorScreen::initTestMode() {
  std::cout << "Initializing test mode..." << std::endl;
  if (m_countdownFont) {
    std::cout << "Font texture ID: " << m_countdownFont->getTextureID() << std::endl;
  }
  // Save object states before entering test mode
  m_savedObjectStates.clear();
  const auto& existingObjects = m_objectManager->getPlacedObjects();
  const auto& existingTemplates = m_objectManager->getObjectTemplates();

  for (size_t i = 0; i < existingObjects.size(); i++) {
    const auto& obj = existingObjects[i];
    size_t templateIndex = 0;
    for (size_t j = 0; j < existingTemplates.size(); j++) {
      if (existingTemplates[j]->getDisplayName() == obj->getDisplayName()) {
        templateIndex = j;
        break;
      }
    }
    m_savedObjectStates.push_back({ templateIndex, obj->getPosition() });
  }

  // Check for start line first
  if (!m_track || !m_track->getStartLineNode()) {
    m_showNoStartLineMessage = true;
    m_messageTimer = MESSAGE_DURATION;
    return;
  }

  // Get start positions before creating any cars
  auto startPositions = m_track->calculateStartPositions();
  if (startPositions.empty()) {
    std::cout << "Cannot start test mode: No valid start positions!\n";
    return;
  }

  // Store editor camera state before switching
  m_editorCameraPos = m_camera.getPosition();
  m_editorCameraScale = m_camera.getScale();
  m_testMode = true;

  // Initialize debug drawer
  m_debugDraw = std::make_unique<DebugDraw>();
  m_debugDraw->init();

  // Initialize physics system
  m_physicsSystem = std::make_unique<PhysicsSystem>();
  m_physicsSystem->init(0.0f, 0.0f);

  // Save object placement state before entering test mode
  m_savedObjectPlacementMode = m_objectPlacementMode;
  m_savedTemplateIndex = m_selectedTemplateIndex;

  // Disable object placement mode
  m_objectPlacementMode = false;
  m_selectedTemplateIndex = -1;

  // Create barrier collisions with physics world
  if (m_physicsSystem) {
    m_levelRenderer->createBarrierCollisions(m_track.get(), m_physicsSystem->getWorld());
  }

  // Create physics bodies for all existing objects
  if (m_objectManager) {
    const auto& existingObjects = m_objectManager->getPlacedObjects();
    const auto& existingTemplates = m_objectManager->getObjectTemplates();

    // Create a new object manager with the physics system
    auto newManager = std::make_unique<ObjectManager>(m_track.get(), m_physicsSystem.get());

    // Copy templates first
    for (const auto& tmpl : existingTemplates) {
      newManager->addTemplate(std::make_unique<PlaceableObject>(*tmpl));
    }

    // Transfer existing objects
    for (const auto& obj : existingObjects) {
      size_t templateIndex = 0;
      for (size_t i = 0; i < existingTemplates.size(); i++) {
        if (existingTemplates[i]->getDisplayName() == obj->getDisplayName()) {
          templateIndex = i;
          break;
        }
      }
      newManager->addObject(templateIndex, obj->getPosition());
    }

    m_objectManager = std::move(newManager);

    // Initialize countdown
    if (!m_countdownFont || !m_countdownFont->getTextureID()) {
      std::cerr << "Font not properly initialized!" << std::endl;
      return;
    }

    m_readyToStart = true;
    m_enableAI = false;
    m_inputEnabled = false;

  }

  // Load car texture
  m_carTexture = JAGEngine::ResourceManager::getTexture("Textures/car.png").id;

  // Generate rainbow colors for cars
  int numCars = startPositions.size();
  std::vector<JAGEngine::ColorRGBA8> carColors(numCars);
  for (int i = 0; i < numCars; i++) {
    // Convert index to hue (0 to 1)
    float hue = float(i) / float(numCars);
    float saturation = 0.8f;  // Slightly reduced for better visibility
    float value = 1.0f;

    // Convert HSV to RGB
    float c = value * saturation;
    float x = c * (1 - std::abs(std::fmod(hue * 6, 2.0f) - 1));
    float m = value - c;

    float r = 0, g = 0, b = 0;
    if (hue < 1.0f / 6.0f) { r = c; g = x; b = 0; }
    else if (hue < 2.0f / 6.0f) { r = x; g = c; b = 0; }
    else if (hue < 3.0f / 6.0f) { r = 0; g = c; b = x; }
    else if (hue < 4.0f / 6.0f) { r = 0; g = x; b = c; }
    else if (hue < 5.0f / 6.0f) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    carColors[i] = JAGEngine::ColorRGBA8(
      uint8_t((r + m) * 255),
      uint8_t((g + m) * 255),
      uint8_t((b + m) * 255),
      255
    );
  }

  // Create cars for each position
  for (size_t i = 0; i < startPositions.size(); i++) {
    const auto& pos = startPositions[i];
    b2BodyId carBody = m_physicsSystem->createDynamicBody(pos.position.x, pos.position.y);

    // Cars should collide with everything using the combined mask
    uint16_t carCategory = PhysicsSystem::CATEGORY_CAR;
    uint16_t carMask = PhysicsSystem::MASK_CAR;

    m_physicsSystem->createPillShape(carBody, 15.0f, 15.0f,
      carCategory, carMask, PhysicsSystem::CollisionType::DEFAULT);

    // Adjusted physics properties for better car-to-car collisions
    b2Body_SetLinearDamping(carBody, 0.8f);     // Increased for more stability
    b2Body_SetAngularDamping(carBody, 3.0f);    // Increased to prevent excessive spinning

    // Create mass data with adjusted properties
    b2MassData massData;
    massData.mass = 15.0f;
    massData.center = b2Vec2_zero;
    massData.rotationalInertia = 2.5f;  // Slightly increased for more stability
    b2Body_SetMassData(carBody, massData);

    // Rest of car setup remains the same
    auto car = std::make_unique<Car>(carBody);
    car->setTrack(m_track.get());
    car->resetPosition({ pos.position.x, pos.position.y }, pos.angle);
    car->setProperties(Car::CarProperties());
    car->setColor(carColors[i]);

    // Create AI driver for all cars except the first one (player)
    if (i > 0) {
      auto aiDriver = std::make_unique<AIDriver>(car.get());
      // Set initial AI configuration
      aiDriver->setConfig(m_aiConfig);
      // Set ObjectManager for this specific AI driver
      aiDriver->setObjectManager(m_objectManager.get());
      std::cout << "Set object manager for AI car " << i << " (manager addr: "
        << m_objectManager.get() << ")\n";
      m_aiDrivers.push_back(std::move(aiDriver));
    }

    m_testCars.push_back(std::move(car));

    // Initialize tracking info
    m_carTrackingInfo.push_back(CarTrackingInfo{
        glm::vec2(0.0f),
        0.0f,
        static_cast<int>(i + 1),
        i == 0
    });
  }

  if (m_objectManager) {
    m_objectManager->setCars(m_testCars);
    for (auto& aiDriver : m_aiDrivers) {
      aiDriver->setObjectManager(m_objectManager.get());
    }
  }

  // Initialize AI config with default values
  m_aiConfig.lookAheadDistance = 100.0f;
  m_aiConfig.centeringForce = 1.0f;
  m_aiConfig.turnAnticipation = 1.0f;
  m_aiConfig.reactionTime = 0.1f;

  initializeAIDrivers();

  m_levelRenderer->setCarTexture(m_carTexture);
  m_levelRenderer->setCars(m_testCars);
  m_camera.setScale(m_testCameraScale);
}

void LevelEditorScreen::drawHUD() {
  if (!m_raceCountdown || !m_countdownFont) {
    std::cout << "Missing font or countdown" << std::endl;
    return;
  }

  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  int screenWidth = game->getWindow().getScreenWidth();
  int screenHeight = game->getWindow().getScreenHeight();

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_textRenderingProgram.use();

  glActiveTexture(GL_TEXTURE0);

  GLuint fontTextureID = m_countdownFont->getTextureID();
  std::cout << "Drawing HUD - Font texture ID: " << fontTextureID << std::endl;
  glBindTexture(GL_TEXTURE_2D, fontTextureID);

  GLint textureUniform = m_textRenderingProgram.getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  m_hudCamera.update();

  glm::mat4 projectionMatrix = m_hudCamera.getCameraMatrix();

  GLint pUniform = m_textRenderingProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &(projectionMatrix[0][0]));

  // Debug projection matrix
  std::cout << "Projection Matrix:\n";
  for (int i = 0; i < 4; i++) {
    std::cout << projectionMatrix[i].x << ", "
      << projectionMatrix[i].y << ", "
      << projectionMatrix[i].z << ", "
      << projectionMatrix[i].w << std::endl;
  }

  // Debug the shader state
  GLint currentProgram;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  std::cout << "Current shader program: " << currentProgram
    << " (expected: " << m_textRenderingProgram.getProgramID() << ")" << std::endl;

  m_hudSpriteBatch.begin();

  std::string countdownText = m_raceCountdown->getText();
  if (!countdownText.empty()) {
    // Keep screen center position
    glm::vec2 textPos(screenWidth / 2.0f, screenHeight / 2.0f);

    // Skip world conversion since it might be causing the scaling issue
    m_countdownFont->draw(m_hudSpriteBatch,
      countdownText.c_str(),
      textPos,
      glm::vec2(6.0f),              // Match Zombie game's scale
      0.0f,
      JAGEngine::ColorRGBA8(255, 0, 0, 255),
      JAGEngine::Justification::MIDDLE);

    std::cout << "Drawing at: " << textPos.x << ", " << textPos.y
      << " with scale 6.0" << std::endl;
  }

  m_hudSpriteBatch.end();
  m_hudSpriteBatch.renderBatch();

  m_textRenderingProgram.unuse();

  // Check for GL errors
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cout << "GL Error in drawHUD: 0x" << std::hex << err << std::dec << std::endl;
  }
}

void LevelEditorScreen::drawTestMode() {
  if (!m_testMode || m_testCars.empty()) return;

  // Draw cars and other game objects
  m_levelRenderer->getTextureProgram().use();
  glm::mat4 cameraMatrix = m_camera.getCameraMatrix();
  GLint pUniform = m_levelRenderer->getTextureProgram().getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &cameraMatrix[0][0]);

  m_spriteBatch.begin();
  // Draw all cars with transparency
  for (const auto& car : m_testCars) {
    b2BodyId bodyId = car->getDebugInfo().bodyId;
    if (b2Body_IsValid(bodyId)) {
      b2Vec2 position = b2Body_GetPosition(bodyId);
      float angle = b2Rot_GetAngle(b2Body_GetRotation(bodyId));
      float carWidth = 20.0f;
      float carHeight = 10.0f;
      glm::vec4 destRect(
        position.x - carWidth / 2.0f,
        position.y - carHeight / 2.0f,
        carWidth,
        carHeight
      );
      glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
      JAGEngine::ColorRGBA8 color = car->getColor();
      color.a = 105;
      m_spriteBatch.draw(destRect, uvRect, m_carTexture, 0.0f, color, angle);
    }
  }
  m_spriteBatch.end();
  m_spriteBatch.renderBatch();
  m_levelRenderer->getTextureProgram().unuse();

  // Draw debug stuff if enabled
  if (m_showDebugDraw && m_physicsSystem) {
    b2WorldId worldId = m_physicsSystem->getWorld();
    m_debugDraw->drawWorld(worldId, cameraMatrix);
    for (const auto& car : m_testCars) {
      m_debugDraw->drawWheelColliders(*car, cameraMatrix);
    }
  }

  if (m_showTrackingPoints) {
    drawCarTrackingDebug();
  }

  // Draw the HUD (including countdown text)
  drawHUD();
}

void LevelEditorScreen::handleCarSelection() {
  if (!m_testMode || m_testCars.empty() || ImGui::GetIO().WantCaptureMouse) return;

  JAGEngine::InputManager& inputManager = m_game->getInputManager();
  glm::vec2 mousePos = m_camera.convertScreenToWorld(inputManager.getMouseCoords());

  // Handle mouse click for selection
  if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {  // Changed from SDLK_SPACE to SDL_BUTTON_LEFT
    float closestDistSq = 1000.0f;  // Squared distance threshold
    size_t newSelectedIndex = m_selectedCarIndex;
    bool foundNewSelection = false;

    // Find closest car to mouse position
    for (size_t i = 0; i < m_testCars.size(); i++) {
      const auto& car = m_testCars[i];
      auto debugInfo = car->getDebugInfo();
      glm::vec2 carPos(debugInfo.position);

      float distSq = glm::length2(mousePos - carPos);

      // Use a reasonable selection radius (adjusted for camera scale)
      float selectionRadius = 30.0f / m_camera.getScale();
      if (distSq < selectionRadius * selectionRadius && distSq < closestDistSq) {
        closestDistSq = distSq;
        newSelectedIndex = i;
        foundNewSelection = true;
      }
    }

    // Only change selection if we found a car and we're not already dragging
    if (foundNewSelection && !m_isDragging && !m_isDraggingObject) {
      // If sync is on and trying to select player car, disable sync
      if (m_syncAllAICars && newSelectedIndex == 0) {
        m_syncAllAICars = false;
      }
      m_selectedCarIndex = newSelectedIndex;
      std::cout << "Selected car " << m_selectedCarIndex << std::endl;
    }
  }

  // Update hover state
  float closestDistSq = 1000.0f;
  m_hoveredCar = nullptr;

  for (const auto& car : m_testCars) {
    auto debugInfo = car->getDebugInfo();
    glm::vec2 carPos(debugInfo.position);

    float distSq = glm::length2(mousePos - carPos);
    float hoverRadius = 30.0f / m_camera.getScale();

    if (distSq < hoverRadius * hoverRadius && distSq < closestDistSq) {
      closestDistSq = distSq;
      m_hoveredCar = car.get();
    }
  }
}

void LevelEditorScreen::drawCarPropertiesUI() {
  if (!m_testMode || m_testCars.empty()) return;

  ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 310, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Car Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
    // Car selection info
    std::string carLabel;
    if (m_syncAllAICars) {
      carLabel = "All AI Cars";
      // If player car is selected while sync is enabled, force selection to first AI car
      if (m_selectedCarIndex == 0 && m_testCars.size() > 1) {
        m_selectedCarIndex = 1;
      }
    }
    else {
      carLabel = (m_selectedCarIndex == 0) ? "Player Car" : "AI Car " + std::to_string(m_selectedCarIndex);
    }

    ImGui::Text("Left click on cars to select them");

    const auto& selectedColor = m_testCars[m_selectedCarIndex]->getColor();
    ImGui::TextColored(ImVec4(selectedColor.r / 255.0f, selectedColor.g / 255.0f,
      selectedColor.b / 255.0f, selectedColor.a / 255.0f),
      "Selected: %s", carLabel.c_str());

    if (m_hoveredCar) {
      for (size_t i = 0; i < m_testCars.size(); i++) {
        if (m_testCars[i].get() == m_hoveredCar) {
          std::string hoverLabel = (i == 0) ? "Player Car" : "AI Car " + std::to_string(i);
          ImGui::TextColored(ImVec4(1, 1, 0, 1), "Hover: %s (Press SPACE to select)", hoverLabel.c_str());
          break;
        }
      }
    }

    // AI Cars Sync Toggle
    if (m_testCars.size() > 1) {
      bool prevSync = m_syncAllAICars;
      if (ImGui::Checkbox("Sync All AI Cars", &m_syncAllAICars)) {
        if (!prevSync && m_syncAllAICars) {
          // If we're enabling sync, make sure an AI car is selected
          if (m_selectedCarIndex == 0) {
            m_selectedCarIndex = 1;  // Select first AI car
          }
          // Apply current AI car properties to all other AI cars
          applySyncedProperties(m_testCars[m_selectedCarIndex]->getProperties());
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("When enabled, all AI cars will share the same properties\nPlayer car remains independent");
      }
    }

    if (ImGui::CollapsingHeader("Car Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
      Car::CarProperties& props = m_testCars[m_selectedCarIndex]->getProperties();
      Car::CarProperties prevProps = props;

      // Movement Properties - Converted to SliderFloat
      ImGui::SliderFloat("Max Speed", &props.maxSpeed, 200.0f, 4000.0f, "%.1f");
      ImGui::SliderFloat("Acceleration", &props.acceleration, 5000.0f, 40000.0f, "%.0f");
      ImGui::SliderFloat("Turn Speed", &props.turnSpeed, 5.0f, 40.0f, "%.1f");
      ImGui::SliderFloat("Drag Factor", &props.dragFactor, 0.9f, 1.0f, "%.3f");
      ImGui::SliderFloat("Turn Reset Rate", &props.turnResetRate, 0.5f, 2.0f, "%.2f");
      ImGui::SliderFloat("Max Angular Velocity", &props.maxAngularVelocity, 1.0f, 5.0f, "%.1f");
      ImGui::SliderFloat("Braking Force", &props.brakingForce, 0.1f, 2.0f, "%.2f");
      ImGui::SliderFloat("Min Speed For Turn", &props.minSpeedForTurn, 0.1f, 5.0f, "%.1f");

      // Surface Effects section
      ImGui::Separator();
      ImGui::Text("Surface Effects");

      ImGui::SliderFloat("Friction Imbalance Sensitivity", &props.frictionImbalanceSensitivity,
        0.0f, 10.0f, "%.2f");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("0 = No steering effect from surface, 10 = Maximum effect");
      }

      ImGui::SliderFloat("Surface Drag Sensitivity", &props.surfaceDragSensitivity,
        0.0f, 3.0f, "%.2f");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("0 = No speed loss on surfaces\n1 = Half effect\n2 = Normal effect\n3 = Stronger effect");
      }

      // Drift Properties
      ImGui::Text("Drift State: %.2f", props.driftState);
      ImGui::SliderFloat("Wheel Grip", &props.wheelGrip, 0.0f, 1.0f, "%.2f");
      ImGui::SliderFloat("Drift Decay Rate", &props.driftDecayRate, 0.1f, 2.0f, "%.2f");

      // Friction Settings
      ImGui::Separator();
      ImGui::Text("Friction Settings");
      ImGui::SliderFloat("Wheel Friction", &props.wheelFriction, 0.1f, 2.0f, "%.2f");
      ImGui::SliderFloat("Surface Friction", &props.baseFriction, 0.1f, 2.0f, "%.2f");

      // When properties change and sync is enabled
      if (memcmp(&prevProps, &props, sizeof(Car::CarProperties)) != 0 && m_syncAllAICars) {
        applySyncedProperties(props);
      }

      if (ImGui::Button("Reset Car Position")) {
        m_testCars[m_selectedCarIndex]->resetPosition();
      }
    }

    // Wheel state information
    if (ImGui::CollapsingHeader("Wheel States", ImGuiTreeNodeFlags_DefaultOpen)) {
      const auto& wheelStates = m_testCars[m_selectedCarIndex]->getWheelStates();
      const char* wheelNames[] = { "Front Left", "Front Right", "Back Left", "Back Right" };

      for (size_t i = 0; i < wheelStates.size(); i++) {
        const auto& state = wheelStates[i];
        ImVec4 color;
        switch (state.surface) {
        case WheelCollider::Surface::Road:
          color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
          break;
        case WheelCollider::Surface::RoadOffroad:
          color = ImVec4(0.7f, 1.0f, 0.0f, 1.0f);
          break;
        case WheelCollider::Surface::Offroad:
          color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
          break;
        case WheelCollider::Surface::OffroadGrass:
          color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
          break;
        case WheelCollider::Surface::Grass:
          color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
          break;
        }

        ImGui::TextColored(color, "%s: %s (Friction: %.2f)",
          wheelNames[i],
          WheelCollider::getSurfaceName(state.surface),
          state.frictionMultiplier);
      }
    }
  }

  if (ImGui::CollapsingHeader("Sensor Data", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_selectedCarIndex > 0 && m_selectedCarIndex - 1 < m_aiDrivers.size()) {
      // Get sensor data from the AI driver
      const auto& sensorData = m_aiDrivers[m_selectedCarIndex - 1]->getSensorData();

      if (sensorData.readings.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No objects detected");
      }
      else {
        ImGui::Text("Detected Objects: %zu", sensorData.readings.size());

        if (ImGui::BeginTable("SensorReadings", 3, ImGuiTableFlags_Borders)) {
          ImGui::TableSetupColumn("Object Type");
          ImGui::TableSetupColumn("Distance");
          ImGui::TableSetupColumn("Side");
          ImGui::TableHeadersRow();

          for (const auto& reading : sensorData.readings) {
            ImGui::TableNextRow();

            // Object Type
            ImGui::TableNextColumn();
            if (reading.object) {
              ImGui::Text("%s", reading.object->getDisplayName().c_str());
            }
            else if (reading.car) {
              ImGui::Text("Car");
            }

            // Distance
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", reading.distance);

            // Side
            ImGui::TableNextColumn();
            ImGui::Text("%s", reading.isLeftSide ? "Left" : "Right");

            // Color the row based on distance
            float distanceRatio = reading.distance / AIDriver::SensorData::SENSOR_RANGE;
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
              ImGui::GetColorU32(ImVec4(
                distanceRatio,
                1.0f - distanceRatio,
                0.0f,
                0.3f)));
          }
          ImGui::EndTable();
        }
      }
    }
    else if (m_selectedCarIndex == 0) {
      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Sensor data only available for AI cars");
    }
  }

  ImGui::End();
}

void LevelEditorScreen::applySyncedProperties(const Car::CarProperties& props) {
  // Only apply to AI cars (skip player car)
  for (size_t i = 1; i < m_testCars.size(); i++) {
    Car::CarProperties newProps = props;
    m_testCars[i]->setProperties(newProps);
  }
}

void LevelEditorScreen::updateAIDrivers() {
  if (!m_testMode || !m_enableAI) return;

  const float timeStep = 1.0f / 60.0f;

  // Update only a subset of AI drivers each frame
  for (size_t i = 0; i < AI_UPDATES_PER_FRAME && i < m_aiDrivers.size(); i++) {
    size_t index = (m_currentAIUpdateIndex + i) % m_aiDrivers.size();
    m_aiDrivers[index]->update(timeStep);
  }
  m_currentAIUpdateIndex = (m_currentAIUpdateIndex + AI_UPDATES_PER_FRAME) % m_aiDrivers.size();
}

void LevelEditorScreen::initializeAIDrivers() {
  // update their configurations
  if (m_aiConfig.lookAheadDistance == 0.0f) {
    m_aiConfig.lookAheadDistance = 100.0f;
    m_aiConfig.centeringForce = 1.0f;
    m_aiConfig.turnAnticipation = 1.0f;
    m_aiConfig.reactionTime = 0.1f;
  }

  // Apply config to existing AI drivers
  for (auto& aiDriver : m_aiDrivers) {
    aiDriver->setConfig(m_aiConfig);
  }
}

void LevelEditorScreen::drawAIDebugUI() {
  if (ImGui::Begin("AI Settings", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::Checkbox("Enable AI", &m_enableAI);

    if (m_enableAI) {
      ImGui::SliderFloat("Look Ahead Distance", &m_aiConfig.lookAheadDistance, 50.0f, 200.0f);
      ImGui::SliderFloat("Centering Force", &m_aiConfig.centeringForce, 0.0f, 2.0f);
      ImGui::SliderFloat("Turn Anticipation", &m_aiConfig.turnAnticipation, 0.0f, 2.0f);
      ImGui::SliderFloat("Reaction Time", &m_aiConfig.reactionTime, 0.05f, 0.5f);

      // Apply config to all AI drivers
      for (auto& aiDriver : m_aiDrivers) {
        aiDriver->setConfig(m_aiConfig);
      }
    }
  }
  ImGui::End();
}

void LevelEditorScreen::drawTestModeUI() {
  if (!m_testMode || m_testCars.empty()) return;

  // Test mode window
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);

  ImGui::Begin("Test Mode", nullptr, ImGuiWindowFlags_NoCollapse);

  if (m_readyToStart && !m_raceCountdown->isCountingDown() && !m_raceCountdown->hasFinished()) {
    if (ImGui::Button("Begin Race", ImVec2(180, 30))) {
      m_raceCountdown->startCountdown();
    }
  }

  if (ImGui::Button("Return to Editor", ImVec2(180, 30))) {
    cleanupTestMode();
  }

  // Debug visualization toggles
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1, 1, 0, 1), "Debug Visualization");
  ImGui::Checkbox("Show Collision Shapes", &m_showDebugDraw);
  ImGui::Checkbox("Show Track Points", &m_showTrackingPoints);

  // Camera controls
  ImGui::Separator();
  ImGui::SliderFloat("Look Ahead Distance", &m_lookAheadDistance, 50.0f, 300.0f);
  ImGui::SliderFloat("Min Screen Edge Distance", &m_minCarScreenDistance, 50.0f, 200.0f);
  ImGui::Text("Use Q/E to zoom");
  ImGui::End();

  // Car controls window and AI settings
  drawCarPropertiesUI();
  drawAIDebugUI();
}

void LevelEditorScreen::cleanupTestMode() {
  // Clean up physics and test mode resources
  m_levelRenderer->clearCars();
  m_objectManager->clearCars();
  bool savedShowStartPositions = m_showStartPositions;

  if (m_physicsSystem) {
    m_levelRenderer->cleanupBarrierCollisions(m_physicsSystem->getWorld());
  }

  // Restore camera state
  m_camera.setPosition(m_editorCameraPos);
  m_camera.setScale(m_editorCameraScale);

  m_carTrackingInfo.clear();
  m_testCars.clear();
  m_aiDrivers.clear();

  if (m_physicsSystem) {
    m_physicsSystem->cleanup();
    m_physicsSystem.reset();
  }

  if (m_debugDraw) {
    m_debugDraw.reset();
  }

  // Restore object placement mode
  m_objectPlacementMode = m_savedObjectPlacementMode;
  m_selectedTemplateIndex = m_savedTemplateIndex;

  // Create new object manager and restore objects to their saved positions
  auto newManager = std::make_unique<ObjectManager>(m_track.get(), nullptr);

  // Copy templates
  const auto& existingTemplates = m_objectManager->getObjectTemplates();
  for (const auto& tmpl : existingTemplates) {
    newManager->addTemplate(std::make_unique<PlaceableObject>(*tmpl));
  }

  // Restore objects using saved states
  for (const auto& state : m_savedObjectStates) {
    newManager->addObject(state.first, state.second);
  }

  m_objectManager = std::move(newManager);

  m_showStartPositions = savedShowStartPositions;
  m_levelRenderer->setShowStartPositions(m_showStartPositions);

  // Reset countdown and ready state
  if (m_raceCountdown) {
    m_raceCountdown->reset();
  }
  m_readyToStart = false;
  m_enableAI = false;
  m_inputEnabled = false;

  m_testMode = false;
}

LevelEditorScreen::CarTrackingInfo LevelEditorScreen::calculateCarTrackingInfo(const Car* car) const {
  CarTrackingInfo info;
  if (!car || !m_track) return info;

  // Get car position
  b2BodyId bodyId = car->getDebugInfo().bodyId;
  if (!b2Body_IsValid(bodyId)) return info;

  b2Vec2 carPos = b2Body_GetPosition(bodyId);
  glm::vec2 carPosition(carPos.x, carPos.y);
  float minDistSq = FLT_MAX;

  // Get spline points
  auto splinePoints = m_track->getSplinePoints(200);
  if (splinePoints.empty()) return info;

  // Find closest segment and interpolated point
  float minDist = FLT_MAX;
  size_t closestIdx = 0;
  glm::vec2 closestPoint = splinePoints[0].position;

  // Check each segment
  for (size_t i = 0; i < splinePoints.size(); i++) {
    size_t nextIdx = (i + 1) % splinePoints.size();
    glm::vec2 segStart = splinePoints[i].position;
    glm::vec2 segEnd = splinePoints[nextIdx].position;
    glm::vec2 segDir = segEnd - segStart;
    float segLengthSq = glm::dot(segDir, segDir);

    if (segLengthSq > 0.0001f) {  // Avoid zero-length segments
      // Calculate projection of car onto segment
      float t = glm::dot(carPosition - segStart, segDir) / segLengthSq;
      t = glm::clamp(t, 0.0f, 1.0f);

      // Get interpolated point on segment
      glm::vec2 projectedPoint = segStart + t * segDir;

      // Check if this is the closest point so far
      float distSq = glm::distance2(carPosition, projectedPoint);
      if (distSq < minDistSq) {
        minDistSq = distSq;
        closestIdx = i;
        closestPoint = projectedPoint;
      }
    }
  }

  info.closestSplinePoint = closestPoint;

  // Calculate accurate distance along spline
  float totalDist = 0.0f;
  for (size_t i = 0; i < closestIdx; i++) {
    totalDist += glm::distance(splinePoints[i].position,
      splinePoints[i + 1].position);
  }

  // Add partial distance to closest point
  if (closestIdx < splinePoints.size() - 1) {
    totalDist += glm::distance(splinePoints[closestIdx].position,
      info.closestSplinePoint);
  }

  info.distanceAlongSpline = totalDist;

  return info;
}

void LevelEditorScreen::updateCarTrackingInfo() {
  if (m_testCars.empty() || m_carTrackingInfo.empty()) return;

  // Update tracking info for each car
  for (size_t i = 0; i < m_testCars.size(); i++) {
    if (i >= m_carTrackingInfo.size()) break;
    m_carTrackingInfo[i] = calculateCarTrackingInfo(m_testCars[i].get());
  }

  // Sort cars by distance along spline to determine race positions
  std::vector<size_t> indices(m_carTrackingInfo.size());
  std::iota(indices.begin(), indices.end(), 0);

  std::sort(indices.begin(), indices.end(),
    [this](size_t a, size_t b) {
      return m_carTrackingInfo[a].distanceAlongSpline >
        m_carTrackingInfo[b].distanceAlongSpline;
    });

  // Update race positions
  for (size_t i = 0; i < indices.size(); i++) {
    m_carTrackingInfo[indices[i]].racePosition = static_cast<int>(i + 1);
  }
}

void LevelEditorScreen::updateTestMode() {
  if (!m_testMode || m_testCars.empty()) return;

  std::cout << "Test mode update - ObjectManager address: " << m_objectManager.get() << "\n";

  if (m_raceCountdown) {
    m_raceCountdown->update(1.0f / 60.0f);
  }

  // Add safety check
  if (!m_physicsSystem || !m_testCars[0]) {
    cleanupTestMode();
    return;
  }

  // Handle car selection before any other input
  handleCarSelection();

  // Handle test mode zoom controls
  JAGEngine::InputManager& inputManager = m_game->getInputManager();
  if (inputManager.isKeyDown(SDLK_q)) {
    m_testCameraScale /= m_zoomFactor;
    m_testCameraScale = glm::clamp(m_testCameraScale, m_minZoom, m_maxZoom);
    m_camera.setScale(m_testCameraScale);
  }
  if (inputManager.isKeyDown(SDLK_e)) {
    m_testCameraScale *= m_zoomFactor;
    m_testCameraScale = glm::clamp(m_testCameraScale, m_minZoom, m_maxZoom);
    m_camera.setScale(m_testCameraScale);
  }

  // Update physics
  const float timeStep = 1.0f / 60.0f;

  // Update player car with input
  if (!m_testCars.empty() && m_inputEnabled) {
    InputState input;
    input.accelerating = inputManager.isKeyDown(SDL_BUTTON_LEFT) ||
      inputManager.isKeyDown(SDLK_w);
    input.braking = inputManager.isKeyDown(SDL_BUTTON_RIGHT) ||
      inputManager.isKeyDown(SDLK_s);
    input.turningLeft = inputManager.isKeyDown(SDLK_LEFT) ||
      inputManager.isKeyDown(SDLK_a);
    input.turningRight = inputManager.isKeyDown(SDLK_RIGHT) ||
      inputManager.isKeyDown(SDLK_d);
    m_testCars[0]->update(input);
  }

  // Update AI drivers
  if (m_enableAI) {
    updateAIDrivers();
  }

  // Update physics and object positions
  if (m_physicsSystem) {
    m_physicsSystem->update(timeStep);
  }

  if (m_objectManager) {
    m_objectManager->update();
  }

  updateCarTrackingInfo();
  updateTestCamera();
}

void LevelEditorScreen::updateTestCamera() {
  if (m_testCars.empty() || m_carTrackingInfo.empty()) return;

  const Car* playerCar = m_testCars[0].get();
  b2BodyId bodyId = playerCar->getDebugInfo().bodyId;
  if (!b2Body_IsValid(bodyId)) return;

  // Get car position
  b2Vec2 carPos = b2Body_GetPosition(bodyId);
  glm::vec2 carWorldPos(carPos.x, carPos.y);

  // Get lookahead point
  glm::vec2 lookAheadPoint = calculateLookAheadPoint(m_carTrackingInfo[0]);

  // Calculate ideal target position - exactly halfway between car and lookahead
  glm::vec2 targetPos = carWorldPos + (lookAheadPoint - carWorldPos) * 0.5f;

  // Get current camera position
  glm::vec2 currentPos = m_camera.getPosition();

  // Calculate distance to target
  float distToTarget = glm::distance(currentPos, targetPos);

  // Simple constant interpolation - only if we're not too far from target
  float interpolationFactor = 0.15f;  // Fixed smoothing factor

  // If we're far from target, move faster
  if (distToTarget > 300.0f) {
    interpolationFactor = 0.3f;
  }

  // Calculate new position
  glm::vec2 newPos = currentPos + (targetPos - currentPos) * interpolationFactor;

  // Force position to stay between car and lookahead point
  glm::vec2 directionToLookahead = glm::normalize(lookAheadPoint - carWorldPos);
  float distCarToLookahead = glm::distance(carWorldPos, lookAheadPoint);

  // Project camera position onto line between car and lookahead
  float projectedDist = glm::dot(newPos - carWorldPos, directionToLookahead);

  // Clamp position to stay between car and lookahead
  if (projectedDist < 0.0f) {
    newPos = carWorldPos;
  }
  else if (projectedDist > distCarToLookahead) {
    newPos = lookAheadPoint;
  }

  // Update camera position
  m_camera.setPosition(newPos);

  // Handle zoom changes from input
  JAGEngine::InputManager& inputManager = m_game->getInputManager();
  if (inputManager.isKeyDown(SDLK_q)) {
    m_testCameraScale /= m_zoomFactor;
    m_testCameraScale = glm::clamp(m_testCameraScale, m_minZoom, m_maxZoom);
  }
  if (inputManager.isKeyDown(SDLK_e)) {
    m_testCameraScale *= m_zoomFactor;
    m_testCameraScale = glm::clamp(m_testCameraScale, m_minZoom, m_maxZoom);
  }

  // Apply current test camera scale
  m_camera.setScale(m_testCameraScale);
}

void LevelEditorScreen::drawCarTrackingDebug() {
  if (!m_testMode || m_carTrackingInfo.empty() || !m_levelRenderer) return;

  auto& textureProgram = m_levelRenderer->getTextureProgram();
  textureProgram.use();

  glm::mat4 cameraMatrix = m_camera.getCameraMatrix();
  GLint pUniform = textureProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &cameraMatrix[0][0]);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_spriteBatch.begin(JAGEngine::GlyphSortType::FRONT_TO_BACK);

  // Draw points for each car
  for (size_t i = 0; i < m_carTrackingInfo.size() && i < m_testCars.size(); i++) {
    const auto& info = m_carTrackingInfo[i];
    const auto& car = m_testCars[i];

    // Get the car's color and print it
    JAGEngine::ColorRGBA8 pointColor = car->getColor();
    std::cout << "Car " << i << " color: R=" << (int)pointColor.r
      << " G=" << (int)pointColor.g
      << " B=" << (int)pointColor.b
      << " A=" << (int)pointColor.a << std::endl;

    // Draw the tracking point
    const float pointSize = 8.0f;
    glm::vec4 pointRect(
      info.closestSplinePoint.x - pointSize,
      info.closestSplinePoint.y - pointSize,
      pointSize * 2.0f,
      pointSize * 2.0f
    );

    // Draw the colored point
    m_spriteBatch.draw(pointRect,
      glm::vec4(0, 0, 1, 1),  // UV coordinates
      0,                       // No texture
      0.0f,                    // Simple depth
      pointColor);             // Use car's color directly

    // Draw look-ahead point only for player car
    if (i == 0) {
      glm::vec2 lookAheadPoint = calculateLookAheadPoint(info);

      // Make look-ahead point a lighter version of car color
      JAGEngine::ColorRGBA8 lookAheadColor(
        std::min<float>(255, static_cast<int>(pointColor.r * 1.5f)),
        std::min<float>(255, static_cast<int>(pointColor.g * 1.5f)),
        std::min<float>(255, static_cast<int>(pointColor.b * 1.5f)),
        255
      );

      glm::vec4 lookAheadRect(
        lookAheadPoint.x - pointSize,
        lookAheadPoint.y - pointSize,
        pointSize * 2.0f,
        pointSize * 2.0f
      );

      m_spriteBatch.draw(lookAheadRect,
        glm::vec4(0, 0, 1, 1),
        0,
        0.0f,
        lookAheadColor);
    }
  }

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();
  textureProgram.unuse();
}

glm::vec2 LevelEditorScreen::calculateLookAheadPoint(const CarTrackingInfo& carInfo) const {
  if (!m_track) return carInfo.closestSplinePoint;

  auto splinePoints = m_track->getSplinePoints(400);  // Higher resolution for smoother interpolation
  if (splinePoints.empty()) return carInfo.closestSplinePoint;

  // Find the current spline segment
  size_t currentIndex = 0;
  float minDist = FLT_MAX;
  float exactT = 0.0f;  // Interpolation parameter along current segment

  // Find which segment we're on and where exactly on that segment
  for (size_t i = 0; i < splinePoints.size(); i++) {
    size_t nextIdx = (i + 1) % splinePoints.size();
    glm::vec2 segStart = splinePoints[i].position;
    glm::vec2 segEnd = splinePoints[nextIdx].position;
    glm::vec2 segDir = segEnd - segStart;
    float segLengthSq = glm::dot(segDir, segDir);

    if (segLengthSq > 0.0001f) {
      // Project closest point onto this segment
      float t = glm::dot(carInfo.closestSplinePoint - segStart, segDir) / segLengthSq;
      if (t >= 0.0f && t <= 1.0f) {
        glm::vec2 projectedPoint = segStart + segDir * t;
        float dist = glm::distance2(carInfo.closestSplinePoint, projectedPoint);
        if (dist < minDist) {
          minDist = dist;
          currentIndex = i;
          exactT = t;
        }
      }
    }
  }

  // Calculate the target distance we want to look ahead
  float remainingDist = m_lookAheadDistance;

  // First, get to the end of current segment
  size_t idx = currentIndex;
  float t = exactT;
  glm::vec2 currentPos = carInfo.closestSplinePoint;
  glm::vec2 lookAheadPoint = currentPos;

  while (remainingDist > 0.0f && idx < splinePoints.size()) {
    size_t nextIdx = (idx + 1) % splinePoints.size();
    glm::vec2 segStart = splinePoints[idx].position;
    glm::vec2 segEnd = splinePoints[nextIdx].position;
    float segLength = glm::distance(segStart, segEnd);

    // Calculate how far along this segment we need to go
    float distanceAlongSegment = segLength * (1.0f - t);

    if (remainingDist <= distanceAlongSegment) {
      // Our target point is on this segment
      float finalT = t + (remainingDist / segLength);
      lookAheadPoint = glm::mix(segStart, segEnd, finalT);
      break;
    }

    // Move to next segment
    remainingDist -= distanceAlongSegment;
    idx = (idx + 1) % splinePoints.size();
    t = 0.0f;  // Start from beginning of next segment
  }

  return lookAheadPoint;
}


