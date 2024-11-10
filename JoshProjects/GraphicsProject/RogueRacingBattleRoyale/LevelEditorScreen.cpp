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

  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  int screenWidth = game->getWindow().getScreenWidth();
  int screenHeight = game->getWindow().getScreenHeight();

  m_camera.init(screenWidth, screenHeight);
  m_camera.setScale(1.0f);
  m_camera.setPosition(glm::vec2(0.0f));

  // Initialize track
  m_track = std::make_unique<SplineTrack>();
  initDefaultTrack();

  // Initialize level renderer
  m_levelRenderer = std::make_unique<LevelRenderer>();
  m_levelRenderer->init();

  // Initialize object manager
  m_objectManager = std::make_unique<ObjectManager>(m_track.get());

  // Initialize background quad and meshes
  m_levelRenderer->updateRoadMesh(m_track.get());

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

  std::cout << "LevelEditorScreen::onExit() complete\n";
}

void LevelEditorScreen::update() {
  handleInput();
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

  // Update level renderer settings from GUI state
  m_levelRenderer->setGrassColor(m_grassColor);
  m_levelRenderer->setOffroadColor(m_offroadColor);
  m_levelRenderer->setGrassNoiseParams(m_grassNoiseScale, m_grassNoiseIntensity);
  m_levelRenderer->setBarrierColors(m_barrierPrimaryColor, m_barrierSecondaryColor);
  m_levelRenderer->setBarrierPatternScale(m_barrierPatternScale);
  m_levelRenderer->setRoadLOD(m_roadLOD);

  // Set preview node if in add node mode
  if (m_addNodeMode) {
    m_levelRenderer->setPreviewNode(m_previewNodePosition, m_showPreviewNode);
  }
  else {
    m_levelRenderer->setPreviewNode(glm::vec2(0.0f), false);
  }

  // Add these lines for object placement preview
  m_levelRenderer->setObjectPlacementMode(m_objectPlacementMode, m_selectedTemplateIndex);
  if (m_objectPlacementMode) {
    glm::vec2 mousePos = m_camera.convertScreenToWorld(
      m_game->getInputManager().getMouseCoords());
    m_levelRenderer->setPreviewPosition(mousePos);
  }

  // Render the level
  m_levelRenderer->render(m_camera,
    m_track.get(),
    m_objectManager.get(),
    m_showSplinePoints,
    static_cast<LevelRenderer::RoadViewMode>(m_roadViewMode));

  // Draw ImGui windows last
  drawImGui();
  drawDebugWindow();
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
  bool modeChanged = false;
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

  // ====================================
  // Handle keyboard input for camera
  // ====================================
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
    TrackNode newNode(position);
    newNode.setBarrierDistance(glm::vec2(10.0f, 10.0f)); // Set default barrier distance
    nodes.push_back(newNode);
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
  interpolatedBarrierDist.x = std::max(interpolatedBarrierDist.x, 10.0f);
  interpolatedBarrierDist.y = std::max(interpolatedBarrierDist.y, 10.0f);
  newNode.setBarrierDistance(interpolatedBarrierDist);

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
