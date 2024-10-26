// GameplayScreen.cpp
#include "GameplayScreen.h"
#include <SDL/SDL.h>
#include <JAGEngine/IMainGame.h>
#include <GL/glew.h>  

#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>

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

void GameplayScreen::onEntry() {
  std::cout << "GameplayScreen::onEntry()\n";

  try {
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
    float zoom = 0.1f; // Adjust this value to zoom in/out
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
    m_playerCarBody = m_physicsSystem->createDynamicBody(-100.0f, -50.0f);
    m_physicsSystem->createBoxShape(m_playerCarBody, 10.0f, 5.0f); // Make it much bigger for testing

    std::cout << "Initialization complete\n";
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in onEntry: " << e.what() << std::endl;
  }
}

void GameplayScreen::onExit() {
  std::cout << "GameplayScreen::onExit()\n";  // Add debug output

  if (m_physicsSystem) {
    m_physicsSystem->cleanup();
    m_physicsSystem.reset();
  }

  m_trackBodies.clear();
  m_playerCarBody = b2_nullBodyId;
}

void GameplayScreen::draw() {
  checkGLError("start of draw");
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    JAGEngine::ColorRGBA8 color(255, 0, 0, 255);  // Make it red for visibility

    m_spriteBatch.draw(destRect, uvRect, m_carTexture, 0.0f, color, angle);
  }

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();

  m_textureProgram.unuse();

  // Draw a debug quad to verify rendering is working
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(&m_projectionMatrix[0][0]);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Draw a small red square in the center
  glBegin(GL_QUADS);
  glColor3f(1.0f, 0.0f, 0.0f);
  float size = 5.0f;  // Size in world units
  glVertex2f(-size, -size);
  glVertex2f(size, -size);
  glVertex2f(size, size);
  glVertex2f(-size, size);
  glEnd();
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

  if (inputManager.isKeyDown(SDLK_ESCAPE)) {
    game->exitGame();
    return;
  }

  if (b2Body_IsValid(m_playerCarBody)) {
    // Adjustable physics constants
    const float BASE_MOVE_SPEED = 1000.0f;
    const float BASE_TURN_SPEED = 20.0f;
    const float MIN_SPEED_TO_TURN = 0.1f;
    const float TURN_SPEED_MULTIPLIER = 4.0f;
    const float MAX_TURN_SPEED = 100.0f;
    const float SPEED_SQUARED_FACTOR = 0.002f;

    b2Vec2 currentVel = b2Body_GetLinearVelocity(m_playerCarBody);
    float currentSpeed = b2Vec2Length(currentVel);  // Using our helper function

    // Forward/Backward movement
    if (inputManager.isKeyDown(SDLK_UP) || inputManager.isKeyDown(SDLK_w)) {
      float angle = b2Rot_GetAngle(b2Body_GetRotation(m_playerCarBody));
      b2Vec2 force = { cos(angle) * BASE_MOVE_SPEED, sin(angle) * BASE_MOVE_SPEED };
      b2Body_ApplyForceToCenter(m_playerCarBody, force, true);
    }

    if (inputManager.isKeyDown(SDLK_DOWN) || inputManager.isKeyDown(SDLK_s)) {
      float angle = b2Rot_GetAngle(b2Body_GetRotation(m_playerCarBody));
      b2Vec2 force = { -cos(angle) * BASE_MOVE_SPEED, -sin(angle) * BASE_MOVE_SPEED };
      b2Body_ApplyForceToCenter(m_playerCarBody, force, true);
    }

    // Calculate turn speed based on current velocity
    float turnSpeed = 0.0f;
    if (currentSpeed > MIN_SPEED_TO_TURN) {
      // Quadratic scaling of turn speed with velocity
      float speedFactor = currentSpeed * currentSpeed * SPEED_SQUARED_FACTOR;
      turnSpeed = BASE_TURN_SPEED + (speedFactor * TURN_SPEED_MULTIPLIER);

      // Clamp to maximum turn speed
      turnSpeed = std::min(turnSpeed, MAX_TURN_SPEED);

      // Apply steering
      if (inputManager.isKeyDown(SDLK_LEFT) || inputManager.isKeyDown(SDLK_a)) {
        b2Body_ApplyTorque(m_playerCarBody, turnSpeed, true);
      }
      if (inputManager.isKeyDown(SDLK_RIGHT) || inputManager.isKeyDown(SDLK_d)) {
        b2Body_ApplyTorque(m_playerCarBody, -turnSpeed, true);
      }
    }

    // Debug output
    std::cout << "Speed: " << currentSpeed
      << " Turn Speed: " << turnSpeed
      << " Pos: (" << b2Body_GetPosition(m_playerCarBody).x
      << ", " << b2Body_GetPosition(m_playerCarBody).y
      << ") Angle: " << b2Rot_GetAngle(b2Body_GetRotation(m_playerCarBody)) << "\n";
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
