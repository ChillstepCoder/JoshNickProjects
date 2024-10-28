// GameplayScreen.cpp
#include "GameplayScreen.h"
#include <SDL/SDL.h>
#include <JAGEngine/IMainGame.h>
#include <GL/glew.h>  
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>

float clamp(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

GameplayScreen::GameplayScreen() : m_playerCarBody(b2_nullBodyId) {
}

GameplayScreen::~GameplayScreen() {
}

int GameplayScreen::getNextScreenIndex() const {
  return -1;
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
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    SDL_Window* window = m_game->getWindow().getSDLWindow();
    SDL_GLContext gl_context = m_game->getWindow().getGLContext();
    if (ImGui_ImplSDL2_InitForOpenGL(window, gl_context) &&
      ImGui_ImplOpenGL3_Init("#version 130")) {
      m_imguiInitialized = true;
    }

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
    float zoom = 0.2f; // Adjust this value to zoom in/out
    m_projectionMatrix = glm::ortho(
      -screenWidth * zoom,  // left
      screenWidth * zoom,   // right
      -screenHeight * zoom, // bottom
      screenHeight * zoom,  // top
      -1.0f, 1.0f
    );

    // Initialize physics
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_physicsSystem->init(0.0f, 0.0f);

    // Create larger car for testing
    m_playerCarBody = m_physicsSystem->createDynamicBody(-100.0f, -100.0f);
    m_physicsSystem->createBoxShape(m_playerCarBody, 15.0f, 15.0f); // Make it much bigger for testing

    std::cout << "Initialization complete\n";
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in onEntry: " << e.what() << std::endl;
  }
}

void GameplayScreen::onExit() {
  if (!m_isExiting) {
    m_isExiting = true;
    cleanupImGui();
  }

  // Rest of cleanup
  if (m_physicsSystem) {
    m_physicsSystem->cleanup();
    m_physicsSystem.reset();
  }
  m_trackBodies.clear();
  m_playerCarBody = b2_nullBodyId;
}

void GameplayScreen::drawImGui() {
  // Set window size and position
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);

  // Create main menu window
  ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoCollapse);

  if (ImGui::Button("Level Editor", ImVec2(180, 40))) {
    std::cout << "Level Editor clicked\n";
  }

  if (ImGui::Button("Race", ImVec2(180, 40))) {
    std::cout << "Race clicked\n";
    m_showMainMenu = false;
  }

  if (ImGui::Button("Options", ImVec2(180, 40))) {
    std::cout << "Options clicked\n";
  }

  if (ImGui::Button("Exit", ImVec2(180, 40))) {
    exitGame();
  }

  ImGui::End();

}

void GameplayScreen::draw() {
  if (m_isExiting || !m_imguiInitialized) {
    return;
  }

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Start ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  // Draw game world
  m_textureProgram.use();

  GLint pUniform = m_textureProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &m_projectionMatrix[0][0]);

  m_spriteBatch.begin();

  if (b2Body_IsValid(m_playerCarBody)) {
    b2Vec2 position = b2Body_GetPosition(m_playerCarBody);
    float angle = b2Rot_GetAngle(b2Body_GetRotation(m_playerCarBody));

    // Make car much bigger for testing
    float carWidth = 20.0f;
    float carHeight = 10.0f;

    glm::vec4 destRect(
      position.x - carWidth / 2.0f,
      position.y - carHeight / 2.0f,
      carWidth,
      carHeight
    );

    glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    JAGEngine::ColorRGBA8 color(255, 0, 0, 255);  // Red

    m_spriteBatch.draw(destRect, uvRect, m_carTexture, 0.0f, color, angle);
  }
 

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();

  m_textureProgram.unuse();

  // Draw ImGui
  drawImGui();

  // Render ImGui
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameplayScreen::update() {
  const float timeStep = 1.0f / 60.0f;

  // Debug physics state before update
  if (b2Body_IsValid(m_playerCarBody)) {
    b2Vec2 position = b2Body_GetPosition(m_playerCarBody);
    b2Vec2 velocity = b2Body_GetLinearVelocity(m_playerCarBody);
    std::cout << "Before physics update:\n"
      << "Position: (" << position.x << ", " << position.y << ")\n"
      << "Velocity: (" << velocity.x << ", " << velocity.y << ")\n";
  }

  m_physicsSystem->update(timeStep);

  // Debug physics state after update
  if (b2Body_IsValid(m_playerCarBody)) {
    b2Vec2 position = b2Body_GetPosition(m_playerCarBody);
    b2Vec2 velocity = b2Body_GetLinearVelocity(m_playerCarBody);
    std::cout << "After physics update:\n"
      << "Position: (" << position.x << ", " << position.y << ")\n"
      << "Velocity: (" << velocity.x << ", " << velocity.y << ")\n";
  }

  checkInput();
}

// Helper function to calculate vector length (can be at the top of GameplayScreen.cpp)
float b2Vec2Length(const b2Vec2& vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

void GameplayScreen::checkInput() {
  JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
  JAGEngine::InputManager& inputManager = game->getInputManager();

  if (m_isExiting) {
    return;
  }

  // Process ImGui events first
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (m_imguiInitialized) {
      ImGui_ImplSDL2_ProcessEvent(&event);
    }
    if (event.type == SDL_QUIT) {
      exitGame();
      return;
    }
  }

  if (inputManager.isKeyDown(SDLK_ESCAPE)) {
    exitGame();
    return;
  }

  if (b2Body_IsValid(m_playerCarBody)) {
    // Adjustable physics constants
    const float ACCELERATION = 5000.0f;     // Forward acceleration force
    const float MAX_SPEED = 200.0f;         // Maximum forward speed
    const float TURN_SPEED = 4.0f;          // Base turning speed
    const float LATERAL_DAMPING = 0.95f;    // How quickly lateral velocity is reduced
    const float DRAG_FACTOR = 0.99f;        // Air resistance
    const float TURN_RESET_RATE = 0.9f;  // How quickly turning resets
    const float MAX_ANGULAR_VELOCITY = 2.0f;  // Maximum rotation speed

    // Get current state
    b2Vec2 currentVel = b2Body_GetLinearVelocity(m_playerCarBody);
    float currentSpeed = b2Vec2Length(currentVel);
    float angle = b2Rot_GetAngle(b2Body_GetRotation(m_playerCarBody));
    float angularVel = b2Body_GetAngularVelocity(m_playerCarBody);

    // Get forward direction vector
    b2Vec2 forwardDir = { cos(angle), sin(angle) };

    // Calculate forward and lateral velocity components
    float forwardSpeed = currentVel.x * forwardDir.x + currentVel.y * forwardDir.y;
    b2Vec2 forwardVel = { forwardDir.x * forwardSpeed, forwardDir.y * forwardSpeed };
    b2Vec2 lateralVel = { currentVel.x - forwardVel.x, currentVel.y - forwardVel.y };

    // Apply lateral damping (reduce sideways sliding)
    b2Vec2 newVel = {
        forwardVel.x + lateralVel.x * LATERAL_DAMPING,
        forwardVel.y + lateralVel.y * LATERAL_DAMPING
    };

    // Apply drag
    newVel.x *= DRAG_FACTOR;
    newVel.y *= DRAG_FACTOR;

    // Set the modified velocity
    b2Body_SetLinearVelocity(m_playerCarBody, newVel);

    // Forward/Backward movement
    if (inputManager.isKeyDown(SDLK_UP) || inputManager.isKeyDown(SDLK_w)) {
      // Only apply force if under max speed
      if (currentSpeed < MAX_SPEED) {
        b2Vec2 force = { cos(angle) * ACCELERATION, sin(angle) * ACCELERATION };
        b2Body_ApplyForceToCenter(m_playerCarBody, force, true);
      }
    }

    if (inputManager.isKeyDown(SDLK_DOWN) || inputManager.isKeyDown(SDLK_s)) {
      // Braking force
      if (forwardSpeed > 1.0f) {  // Only brake when moving forward
        b2Vec2 force = { -forwardDir.x * ACCELERATION * 0.5f, -forwardDir.y * ACCELERATION * 0.5f };
        b2Body_ApplyForceToCenter(m_playerCarBody, force, true);
      }
    }

    // Turning with auto-reset
    if (abs(forwardSpeed) > 1.0f) {  // Only turn when moving
      float turnFactor = TURN_SPEED * (forwardSpeed / MAX_SPEED);
      float currentAngularVel = b2Body_GetAngularVelocity(m_playerCarBody);
      float targetAngularVel = 0.0f;  // Default to no turning

      if (inputManager.isKeyDown(SDLK_LEFT) || inputManager.isKeyDown(SDLK_a)) {
        targetAngularVel = turnFactor * ACCELERATION;
      }
      else if (inputManager.isKeyDown(SDLK_RIGHT) || inputManager.isKeyDown(SDLK_d)) {
        targetAngularVel = -turnFactor * ACCELERATION;
      }

      // Smoothly transition to target angular velocity
      float newAngularVel;
      if (abs(targetAngularVel) > 0.001f) {
        // If actively turning, move toward target velocity
        newAngularVel = currentAngularVel + (targetAngularVel - currentAngularVel) * 0.1f;
      }
      else {
        // If not turning, decay toward zero
        newAngularVel = currentAngularVel * 0.9f;
      }

      // Clamp maximum angular velocity
      newAngularVel = clamp(newAngularVel, -MAX_ANGULAR_VELOCITY, MAX_ANGULAR_VELOCITY);
      b2Body_SetAngularVelocity(m_playerCarBody, newAngularVel);
    }

    // Debug output
    std::cout << "Speed: " << currentSpeed
      << " Forward Speed: " << forwardSpeed
      << " Lateral: (" << lateralVel.x << ", " << lateralVel.y << ")"
      << " Angle: " << angle << "\n";
  }
}

void GameplayScreen::createTrack() {

  // For now, we'll skip track creation
// We'll add walls and boundaries later
// 
  // Create ground
  //b2BodyId groundBody = m_physicsSystem->createStaticBody(0.0f, -10.0f);
  //m_physicsSystem->createBoxShape(groundBody, 100.0f, 20.0f);
  //m_trackBodies.push_back(groundBody);

  // Add more track elements as needed
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
    cleanupImGui();
    if (m_game) {
      m_game->exitGame();
    }
  }
}
