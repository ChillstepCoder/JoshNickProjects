// GameplayScreen.cpp
#include "GameplayScreen.h"
#include <SDL/SDL.h>
#include <JAGEngine/IMainGame.h>
#include <GL/glew.h>  
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>
#include "LevelEditorScreen.h"


float b2Vec2Length(const b2Vec2& vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

float clamp(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

GameplayScreen::GameplayScreen() :
  m_playerCarBody(b2_nullBodyId),
  m_showMainMenu(true),
  m_showDebugWindow(true),
  m_isExiting(false),
  m_imguiInitialized(false) {
}

GameplayScreen::~GameplayScreen() {
}

int GameplayScreen::getNextScreenIndex() const {
  return 1;  // Index of LevelEditorScreen
}

int GameplayScreen::getPreviousScreenIndex() const {
  return -1;
}

void GameplayScreen::build() {
  // Initialize any resources needed by the screen
  std::cout << "GameplayScreen::build()\n";  // Add debug output
}

void GameplayScreen::destroy() {

}

void GameplayScreen::cleanupImGui() {
  if (m_imguiInitialized) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    m_imguiInitialized = false;
  }
}

void GameplayScreen::onEntry() {
  std::cout << "GameplayScreen::onEntry()\n";

  try {
    // Initialize Shaders
    initShaders();
    m_spriteBatch.init();

    // Load car texture
    m_carTexture = JAGEngine::ResourceManager::getTexture("Textures/car.png").id;
    std::cout << "Loaded car texture with ID: " << m_carTexture << std::endl;

    // Get window dimensions
    JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
    int screenWidth = game->getWindow().getScreenWidth();
    int screenHeight = game->getWindow().getScreenHeight();

    // Simplified projection matrix for 2D rendering
    float zoom = 0.2f;
    m_projectionMatrix = glm::ortho(
      -screenWidth * zoom,
      screenWidth * zoom,
      -screenHeight * zoom,
      screenHeight * zoom,
      -1.0f, 1.0f
    );

    // Initialize physics
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_physicsSystem->init(0.0f, 0.0f);

    // Create car body
    m_playerCarBody = m_physicsSystem->createDynamicBody(-100.0f, -100.0f);
    m_physicsSystem->createPillShape(m_playerCarBody, 15.0f, 15.0f,
      CATEGORY_CAR,  // Category
      CATEGORY_BARRIER | CATEGORY_CAR,  // Mask
      CollisionType::DEFAULT);  // Collision type

    // Initialize default car properties
    m_defaultCarProps = Car::CarProperties();

    // Create the car and initialize with default properties
    m_car = std::make_unique<Car>(m_playerCarBody);
    m_car->setProperties(m_defaultCarProps);

    std::cout << "Initialization complete\n";
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in onEntry: " << e.what() << std::endl;
  }
}

void GameplayScreen::drawDebugWindow() {
  if (!m_showDebugWindow) return;

  ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Debug Controls", &m_showDebugWindow)) {
    if (ImGui::CollapsingHeader("Car Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
      Car::CarProperties& props = m_car->getProperties();

      ImGui::DragFloat("Max Speed", &props.maxSpeed, 1.0f, 200.0f, 4000.0f);
      ImGui::DragFloat("Acceleration", &props.acceleration, 10.0f, 5000.0f, 40000.0f);
      ImGui::DragFloat("Turn Speed", &props.turnSpeed, 0.1f, 5.0f, 40.0f);
      ImGui::DragFloat("Lateral Damping", &props.lateralDamping, 0.01f, 0.5f, 1.0f);
      ImGui::DragFloat("Drag Factor", &props.dragFactor, 0.001f, 0.9f, 1.0f);
      ImGui::DragFloat("Turn Reset Rate", &props.turnResetRate, 0.01f, 0.5f, 2.0f);
      ImGui::DragFloat("Max Angular Velocity", &props.maxAngularVelocity, 0.1f, 1.0f, 5.0f);
      ImGui::DragFloat("Braking Force", &props.brakingForce, 0.1f, 0.1f, 2.0f);
      ImGui::DragFloat("Min Speed For Turn", &props.minSpeedForTurn, 0.1f, 0.1f, 5.0f);

      // Add friction controls
      ImGui::Separator();
      ImGui::Text("Friction Settings");
      ImGui::DragFloat("Wheel Friction", &props.wheelFriction, 0.01f, 0.1f, 2.0f);
      ImGui::DragFloat("Surface Friction", &props.baseFriction, 0.01f, 0.1f, 2.0f);

      if (ImGui::Button("Reset to Defaults")) {
        props = m_defaultCarProps;
      }

      ImGui::Separator();
      if (ImGui::Button("Reset Car Position")) {
        m_car->resetPosition();
      }
    }

    if (ImGui::CollapsingHeader("Debug Info")) {
      Car::DebugInfo debug = m_car->getDebugInfo();
      ImGui::Text("Position: (%.1f, %.1f)", debug.position.x, debug.position.y);
      ImGui::Text("Velocity: (%.1f, %.1f)", debug.velocity.x, debug.velocity.y);
      ImGui::Text("Speed: %.1f", debug.currentSpeed);
      ImGui::Text("Forward Speed: %.1f", debug.forwardSpeed);
      ImGui::Text("Angle: %.2f", debug.angle);
      ImGui::Text("Angular Velocity: %.2f", debug.angularVelocity);
      ImGui::Text("Effective Friction: %.2f", debug.effectiveFriction);
    }
  }
  ImGui::End();
}

void GameplayScreen::onExit() {
  m_isExiting = true;

  // Cleanup physics
  if (m_physicsSystem) {
    m_physicsSystem->cleanup();
    m_physicsSystem.reset();
  }
  m_trackBodies.clear();
  m_playerCarBody = b2_nullBodyId;
}

void GameplayScreen::drawImGui() {
  // Define desired window size
  ImVec2 windowSize(250, 300);

  // Compute centered position for the gameplay menu window
  ImGuiIO& io = ImGui::GetIO();
  ImVec2 displaySize = io.DisplaySize;
  ImVec2 windowPos((displaySize.x - windowSize.x) * 0.5f,
    (displaySize.y - windowSize.y) * 0.5f);

  // Always force the gameplay menu to be centered
  ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

  // Use a unique window title for the gameplay menu so it doesn't share state
  ImGui::Begin("Main Menu (Game)", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

  if (ImGui::Button("Level Editor", ImVec2(230, 40))) {
    LevelEditorScreen::SetNextEditorMode(true);
    m_currentState = JAGEngine::ScreenState::CHANGE_NEXT;
  }

  if (ImGui::Button("Race", ImVec2(230, 40))) {
    std::cout << "Race clicked\n";
    // For Race mode, set editor mode to false so that editor GUI is hidden.
    LevelEditorScreen::SetNextEditorMode(false);
    m_currentState = JAGEngine::ScreenState::CHANGE_NEXT;
  }

  if (ImGui::Button("Options", ImVec2(230, 40))) {
    std::cout << "Options clicked\n";
  }

  if (ImGui::Button("Exit", ImVec2(230, 40))) {
    exitGame();
  }

  ImGui::End();
}

void GameplayScreen::draw() {
  // Clear the screen with a dark background color
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Use your texture shader if needed (omitted here since we won't draw the test car)
  m_textureProgram.use();
  glUniformMatrix4fv(m_textureProgram.getUniformLocation("P"), 1, GL_FALSE, &m_projectionMatrix[0][0]);

  // Begin and immediately end the sprite batch (if nothing is drawn)
  m_spriteBatch.begin();
  // (No test car or other world elements are drawn here.)
  m_spriteBatch.end();
  m_spriteBatch.renderBatch();

  m_textureProgram.unuse();

  // Draw only the main menu ImGui window
  drawImGui();

}

void GameplayScreen::update() {
  const float timeStep = 1.0f / 60.0f;

  // Debug physics state before update
  if (b2Body_IsValid(m_playerCarBody)) {
    b2Vec2 position = b2Body_GetPosition(m_playerCarBody);
    b2Vec2 velocity = b2Body_GetLinearVelocity(m_playerCarBody);
  }

  m_physicsSystem->update(timeStep);

  // Debug physics state after update
  if (b2Body_IsValid(m_playerCarBody)) {
    b2Vec2 position = b2Body_GetPosition(m_playerCarBody);
    b2Vec2 velocity = b2Body_GetLinearVelocity(m_playerCarBody);
  }

  checkInput();
}

void GameplayScreen::checkInput() {
  if (m_isExiting) return;

  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  JAGEngine::InputManager& inputManager = game->getInputManager();

  if (inputManager.isKeyDown(SDLK_ESCAPE)) {
    exitGame();
    return;
  }

  // Create input state for car
  InputState input;
  input.accelerating = inputManager.isKeyDown(SDLK_UP) || inputManager.isKeyDown(SDLK_w);
  input.braking = inputManager.isKeyDown(SDLK_DOWN) || inputManager.isKeyDown(SDLK_s);
  input.turningLeft = inputManager.isKeyDown(SDLK_LEFT) || inputManager.isKeyDown(SDLK_a);
  input.turningRight = inputManager.isKeyDown(SDLK_RIGHT) || inputManager.isKeyDown(SDLK_d);
  input.handbrake = inputManager.isKeyDown(SDLK_SPACE);

  // Update car with input state
  if (m_car) {
    m_car->update(input);
  }
}

void GameplayScreen::createTrack() {

  // For now, we'll skip track creation
  // game play screen is currently just a menu with a car that you can drive around an empty area.
}

void GameplayScreen::initShaders() {
  // Compile shaders
  m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
  m_textureProgram.addAttribute("vertexPosition");
  m_textureProgram.addAttribute("vertexColor");
  m_textureProgram.addAttribute("vertexUV");
  m_textureProgram.linkShaders();
}

void GameplayScreen::checkGLError(const char* location) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error at " << location << ": " << std::hex << err << std::dec << std::endl;
  }
}

void GameplayScreen::exitGame() {
  if (!m_isExiting) {
    m_isExiting = true;
    if (m_game) {
      m_game->exitGame();
    }
  }
}
