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
  m_physicsSystem->update(timeStep);

  // Debug output car position
  if (b2Body_IsValid(m_playerCarBody)) {
    b2Vec2 position = b2Body_GetPosition(m_playerCarBody);
    std::cout << "Car position: (" << position.x << ", " << position.y << ")\n";

    // Debug camera/projection info
    glm::vec4 viewport(0, 0,
      m_game->getWindow().getScreenWidth(),
      m_game->getWindow().getScreenHeight());

    // Convert world coordinates to screen coordinates for debugging
    glm::vec3 screenPos = glm::project(
      glm::vec3(position.x, position.y, 0),
      glm::mat4(1.0f),  // model matrix (identity)
      m_projectionMatrix,
      viewport
    );

    std::cout << "Car screen position: (" << screenPos.x << ", " << screenPos.y << ")\n";
    std::cout << "Viewport: " << viewport.x << ", " << viewport.y << ", "
      << viewport.z << ", " << viewport.w << "\n";
  }

  checkInput();
}

void GameplayScreen::checkInput() {
  const Uint8* keyState = SDL_GetKeyboardState(nullptr);

  if (keyState[SDL_SCANCODE_ESCAPE]) {
    std::cout << "ESC pressed, exiting...\n";
    m_game->exitGame();  // This should properly trigger game exit
  }

  if (b2Body_IsValid(m_playerCarBody)) {
    b2Body_SetAwake(m_playerCarBody, true);
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
